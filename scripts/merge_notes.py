#!/usr/bin/env python3
"""Merge the chapter PDFs of a folder into one PDF with cover, contents, and parts.

Folder layout (inside NOTES_DIR):

  docs/notes/
  ├── 1-Foundations/           a part: one subfolder per part
  │   ├── 00-introduction.pdf
  │   └── 01-arm-cortex-m3-architecture.pdf
  ├── 2-ScratchOS/
  │   ├── 02-elf-and-linking.pdf
  │   └── 03-boot_cpu_config.pdf
  ├── 3-FreeRTOS/
  ├── 9-Appendices/            a folder whose name contains "append" holds appendices
  │   └── A-toolchain.pdf
  └── merged/                  the output, ignored

Parts and chapters are taken in file-name order, so the numeric prefix decides
their position and is removed from the title ("2-ScratchOS" -> "ScratchOS").
PDFs placed directly in NOTES_DIR are chapters without a part, put first.

The merged PDF contains: cover, table of contents (parts, chapters, and the
sections found in the bookmarks of each chapter PDF), a title page for each
part, the chapters, nested bookmarks, and page numbers on every page except
the cover. Chapters are numbered 1, 2, 3, ... across parts; appendices A, B, ...

Export the chapter PDFs WITH bookmarks (outline) and WITHOUT page numbers:
the sections of the contents come from the bookmarks, and the page numbers
are added here, counted over the whole document.

Configuration comes from environment variables (set in the workflow):
  NOTES_DIR      folder with the parts and chapter PDFs  (default: notes)
  OUTPUT         path of the merged PDF                  (default: <NOTES_DIR>/merged/notes.pdf)
  TOC_DEPTH      deepest heading level of the chapters shown in the contents:
                 0 = chapters only, 1 = h1, 2 = h1-h2, ..., 6 = h1-h6 (default: 6)
  LOGO           PNG or JPG logo for the cover           (optional)
  TITLE          course name, the cover title
  COURSE_CODE    course code, e.g. 01WMQWQ               (optional)
  PROGRAM        degree programme                        (optional)
  YEAR           academic year                           (optional)
  PROFESSOR      course instructor                       (optional)
  AUTHOR         author of the notes                     (optional)
"""
import datetime
import io
import os
import re
import string
import sys
from dataclasses import dataclass, field
from pathlib import Path

from pypdf import PdfReader, PdfWriter
from pypdf.annotations import Link
from reportlab.lib.pagesizes import A4
from reportlab.lib.utils import ImageReader
from reportlab.pdfbase.pdfmetrics import stringWidth
from reportlab.pdfgen import canvas

ACRONYMS = {"cpu", "io", "uart", "svc", "rtos", "mmu", "mpu", "dma", "irq", "nvic", "elf", "os", "arm"}
ROMAN = ["I", "II", "III", "IV", "V", "VI", "VII", "VIII", "IX", "X"]

WIDTH, HEIGHT = A4
MARGIN_X, MARGIN_TOP, MARGIN_BOTTOM = 72, 90, 72


def toc_style(level: int) -> tuple:
    """Contents layout of a level: font, size, indent, space taken by the line.

    Level 0 is a part, 1 a chapter, 2 an h1 of the chapter, ..., 7 an h6.
    """
    if level == 0:
        return ("Helvetica-Bold", 13, 0, 30)
    if level == 1:
        return ("Helvetica-Bold", 11.5, 0, 20)
    h = level - 1                                     # heading level, 1..6
    size = max(10.5 - 0.5 * (h - 1), 8.5)
    return ("Helvetica", size, 26 + 16 * (h - 1), size + 5.5)


# --------------------------------------------------------------------------- titles

def pretty(name: str) -> str:
    """'03-device_interrupts_syscalls' -> 'Device Interrupts Syscalls', '2-ScratchOS' -> 'ScratchOS'."""
    stem = re.sub(r"^[0-9]+[-_ ]*", "", name)
    stem = re.sub(r"^[A-Z][-_ ]+", "", stem)          # 'A-toolchain' -> 'toolchain'
    words = re.split(r"[-_ ]+", stem)
    out = []
    for w in words:
        if not w:
            continue
        if w.lower() in ACRONYMS:
            out.append(w.upper())
        elif w.islower():
            out.append(w.capitalize())
        else:
            out.append(w)                             # keep mixed case: ScratchOS, FreeRTOS
    return " ".join(out)


# --------------------------------------------------------------------------- structure

@dataclass
class Chapter:
    path: Path
    title: str
    label: str = ""
    reader: PdfReader = None
    outline: list = field(default_factory=list)       # [{"title", "page", "children"}]
    start: int = 0                                    # index of the first page in the output


@dataclass
class Part:
    title: str | None                                 # None: chapters without a part
    label: str = ""                                   # "Part I", "" for appendices
    appendix: bool = False
    chapters: list = field(default_factory=list)
    page: int = -1                                    # index of the part title page


def read_outline(reader: PdfReader) -> list:
    """Bookmarks of a PDF as a tree, with the page index (inside that PDF) of each one."""
    def walk(items):
        nodes = []
        for item in items:
            if isinstance(item, list):                # children of the previous item
                if nodes:
                    nodes[-1]["children"] = walk(item)
                else:
                    nodes.extend(walk(item))
                continue
            try:
                page = reader.get_destination_page_number(item)
            except Exception:
                continue
            if page is None or page < 0:
                continue
            nodes.append({"title": str(item.title).strip(), "page": page, "children": []})
        return nodes
    try:
        return walk(reader.outline)
    except Exception as e:
        print(f"warning: cannot read the bookmarks: {e}", file=sys.stderr)
        return []


def discover(notes: Path, output: Path) -> list:
    skip = output.parent.resolve()
    groups = []

    loose = sorted(p for p in notes.glob("*.pdf") if p.resolve() != output.resolve())
    if loose:
        groups.append((None, False, loose))

    for folder in sorted(d for d in notes.iterdir() if d.is_dir()):
        if folder.resolve() == skip or folder.name.startswith((".", "_")):
            continue
        pdfs = sorted(folder.glob("*.pdf"))
        if not pdfs:
            continue                                  # an empty part is not printed
        groups.append((pretty(folder.name), "append" in folder.name.lower(), pdfs))

    parts, part_no, chapter_no, appendix_no = [], 0, 0, 0
    for title, appendix, pdfs in groups:
        part = Part(title=title, appendix=appendix)
        if title is not None and not appendix:
            part.label = f"Part {ROMAN[part_no] if part_no < len(ROMAN) else part_no + 1}"
            part_no += 1
        for pdf in pdfs:
            ch = Chapter(path=pdf, title=pretty(pdf.stem))
            if appendix:
                ch.label = string.ascii_uppercase[appendix_no % 26]
                appendix_no += 1
            else:
                chapter_no += 1
                ch.label = str(chapter_no)
            ch.reader = PdfReader(str(pdf))
            ch.outline = read_outline(ch.reader)
            if not ch.outline:
                print(f"warning: {pdf.name} has no bookmarks, only the chapter goes in the contents",
                      file=sys.stderr)
            part.chapters.append(ch)
        parts.append(part)
    return parts


# --------------------------------------------------------------------------- contents

def toc_entries(parts: list, depth: int) -> list:
    """Flat list of (level, label, title, target) where target() gives the page index."""
    entries = []

    def sections(ch, nodes, level):
        if level - 1 > depth:                         # level 2 = h1, ..., level 7 = h6
            return
        for n in nodes:
            entries.append((level, "", n["title"], lambda ch=ch, n=n: ch.start + n["page"]))
            sections(ch, n["children"], level + 1)

    for part in parts:
        if part.title is not None:
            heading = f"{part.label} – {part.title}" if part.label else part.title
            entries.append((0, "", heading, lambda p=part: p.page))
        for ch in part.chapters:
            entries.append((1, ch.label, ch.title, lambda ch=ch: ch.start))
            sections(ch, ch.outline, 2)
    return entries


def paginate(entries: list) -> list:
    """Split the entries into pages; returns, per page, a list of (entry, y)."""
    pages, current = [], []
    y = HEIGHT - MARGIN_TOP - 40                      # room for the "Contents" heading
    for e in entries:
        step = toc_style(e[0])[3]
        if e[0] == 0 and current:
            y -= 8                                    # extra space before a part
        if y - step < MARGIN_BOTTOM:
            pages.append(current)
            current, y = [], HEIGHT - MARGIN_TOP
        y -= step
        current.append((e, y))
    pages.append(current)
    return pages


def fit(text: str, font: str, size: float, room: float) -> str:
    if stringWidth(text, font, size) <= room:
        return text
    while text and stringWidth(text + "…", font, size) > room:
        text = text[:-1]
    return text + "…"


def render_toc(pages: list) -> tuple[bytes, list]:
    """Draw the contents; returns the PDF and the links (toc page, rect, target)."""
    buf = io.BytesIO()
    c = canvas.Canvas(buf, pagesize=A4)
    links = []
    right = WIDTH - MARGIN_X
    for i, page in enumerate(pages):
        if i == 0:
            c.setFont("Helvetica-Bold", 20)
            c.drawString(MARGIN_X, HEIGHT - MARGIN_TOP, "Contents")
        for (level, label, title, target), y in page:
            font, size, indent, _ = toc_style(level)
            number = str(target() + 1)
            x = MARGIN_X + indent
            c.setFont(font, size)
            if label:
                c.drawString(x, y, label)
                x += 26
            num_w = stringWidth(number, font, size)
            text = fit(title, font, size, right - num_w - 24 - x)
            c.drawString(x, y, text)
            c.drawRightString(right, y, number)
            if level > 0:                             # dot leaders
                start = x + stringWidth(text, font, size) + 6
                end = right - num_w - 6
                c.setFont("Helvetica", size)
                dot = stringWidth(". ", "Helvetica", size)
                if end > start:
                    c.drawString(start, y, ". " * int((end - start) / dot))
            links.append((i, (MARGIN_X + indent, y - 3, right, y + size), target()))
        c.showPage()
    c.save()
    return buf.getvalue(), links


# --------------------------------------------------------------------------- pages

def render_cover() -> bytes:
    env = os.environ.get
    title, code, program = env("TITLE", "Course Notes"), env("COURSE_CODE", ""), env("PROGRAM", "")
    year, professor, author, logo = env("YEAR", ""), env("PROFESSOR", ""), env("AUTHOR", ""), env("LOGO", "")

    buf = io.BytesIO()
    c = canvas.Canvas(buf, pagesize=A4)
    width, height = A4
    y = height - 80

    if logo and Path(logo).is_file():
        img = ImageReader(logo)
        iw, ih = img.getSize()
        w = 220
        h = w * ih / iw
        c.drawImage(img, (width - w) / 2, y - h, w, h, mask="auto")
        y -= h + 60
    else:
        if logo:
            print(f"warning: logo {logo} not found, cover without logo", file=sys.stderr)
        y -= 120

    if program:
        c.setFont("Helvetica", 13)
        c.drawCentredString(width / 2, y, program)
        y -= 34
    c.setFont("Helvetica-Bold", 24)
    c.drawCentredString(width / 2, y, title)
    y -= 26
    details = " · ".join(x for x in (code, f"A.Y. {year}" if year else "") if x)
    if details:
        c.setFont("Helvetica", 13)
        c.drawCentredString(width / 2, y, details)
        y -= 40
    c.setFont("Helvetica", 12)
    if professor:
        c.drawCentredString(width / 2, y, f"Professor: {professor}")
        y -= 18
    if author:
        c.drawCentredString(width / 2, y, f"Notes by {author}")
        y -= 18
    c.setFont("Helvetica-Oblique", 10)
    c.drawCentredString(width / 2, y - 4, "Personal study notes, not official course material")

    c.setFont("Helvetica", 9)
    c.drawCentredString(width / 2, 50, f"Generated on {datetime.date.today():%d/%m/%Y}")
    c.showPage()
    c.save()
    return buf.getvalue()


def render_part_page(part: Part) -> bytes:
    buf = io.BytesIO()
    c = canvas.Canvas(buf, pagesize=A4)
    y = HEIGHT / 2 + 40
    if part.label:
        c.setFont("Helvetica", 18)
        c.drawCentredString(WIDTH / 2, y, part.label)
        y -= 40
    c.setFont("Helvetica-Bold", 30)
    c.drawCentredString(WIDTH / 2, y, part.title)
    c.showPage()
    c.save()
    return buf.getvalue()


def stamp_page_numbers(writer: PdfWriter) -> None:
    """Write the page number at the bottom of every page except the cover."""
    buf = io.BytesIO()
    c = canvas.Canvas(buf)
    for i, page in enumerate(writer.pages):
        box = page.mediabox
        left, bottom = float(box.left), float(box.bottom)
        w, h = float(box.width), float(box.height)
        c.setPageSize((left + w, bottom + h))
        if i > 0:
            c.setFont("Helvetica", 9)
            c.drawCentredString(left + w / 2, bottom + 24, str(i + 1))
        c.showPage()
    c.save()
    overlay = PdfReader(io.BytesIO(buf.getvalue()))
    for i, page in enumerate(writer.pages):
        if i > 0:
            page.merge_page(overlay.pages[i])


def add_bookmarks(writer: PdfWriter, parts: list) -> None:
    def nested(ch, nodes, parent):
        for n in nodes:
            item = writer.add_outline_item(n["title"], ch.start + n["page"], parent=parent)
            nested(ch, n["children"], item)

    writer.add_outline_item("Contents", 1)
    for part in parts:
        parent = None
        if part.title is not None:
            name = f"{part.label} – {part.title}" if part.label else part.title
            parent = writer.add_outline_item(name, part.page)
        for ch in part.chapters:
            item = writer.add_outline_item(f"{ch.label}  {ch.title}", ch.start, parent=parent)
            nested(ch, ch.outline, item)


# --------------------------------------------------------------------------- main

def main() -> None:
    notes = Path(os.environ.get("NOTES_DIR", "notes"))
    output = Path(os.environ.get("OUTPUT", notes / "merged" / "notes.pdf"))
    depth = int(os.environ.get("TOC_DEPTH", "6"))

    parts = discover(notes, output)
    if not any(p.chapters for p in parts):
        sys.exit(f"no PDF found in {notes}")

    # 1. Lay out the contents to know how many pages it takes.
    entries = toc_entries(parts, depth)
    toc_pages = paginate(entries)

    # 2. Give every part page and chapter its final page index.
    index = 1 + len(toc_pages)                        # after cover and contents
    for part in parts:
        if part.title is not None:
            part.page = index
            index += 1
        for ch in part.chapters:
            ch.start = index
            index += len(ch.reader.pages)

    # 3. Assemble the document.
    toc_pdf, links = render_toc(toc_pages)
    writer = PdfWriter()
    writer.append(PdfReader(io.BytesIO(render_cover())))
    writer.append(PdfReader(io.BytesIO(toc_pdf)))
    for part in parts:
        if part.title is not None:
            writer.append(PdfReader(io.BytesIO(render_part_page(part))))
        for ch in part.chapters:
            writer.append(ch.reader, import_outline=False)
    assert len(writer.pages) == index, "page count differs from the layout"

    # 4. Clickable contents, bookmarks, page numbers.
    for toc_page, rect, target in links:
        writer.add_annotation(page_number=1 + toc_page,
                              annotation=Link(rect=rect, target_page_index=target, border=[0, 0, 0]))
    add_bookmarks(writer, parts)
    stamp_page_numbers(writer)

    output.parent.mkdir(parents=True, exist_ok=True)
    with open(output, "wb") as f:
        writer.write(f)
    n_ch = sum(len(p.chapters) for p in parts)
    print(f"{output}: {n_ch} chapters in {len(parts)} groups, "
          f"{len(toc_pages)} contents page(s), {len(writer.pages)} pages")


if __name__ == "__main__":
    main()
