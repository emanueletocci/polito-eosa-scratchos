#!/usr/bin/env python3
"""Merge the chapter PDFs of a folder into one PDF with a cover page.

The chapters are taken in file-name order (01-…, 02-…), so the numeric prefix
decides their position. Each chapter gets a bookmark in the merged PDF.

Configuration comes from environment variables (set in the workflow):
  NOTES_DIR      folder with the chapter PDFs            (default: notes)
  OUTPUT         path of the merged PDF                  (default: <NOTES_DIR>/merged/notes.pdf)
  LOGO           PNG or JPG logo for the cover           (optional)
  TITLE          course name, the cover title
  COURSE_CODE    course code, e.g. 01WMQWQ               (optional)
  PROGRAM        degree programme                        (optional)
  YEAR           academic year                           (optional)
  PROFESSOR      course instructor                       (optional)
  AUTHOR         author of the notes                     (optional)
"""
import datetime
import os
import re
import sys
from pathlib import Path

from pypdf import PdfReader, PdfWriter
from reportlab.lib.pagesizes import A4
from reportlab.lib.utils import ImageReader
from reportlab.pdfgen import canvas

ACRONYMS = {"cpu", "io", "uart", "svc", "rtos", "mmu", "mpu", "dma", "irq", "nvic", "elf", "os"}


def chapter_title(path: Path) -> str:
    """'03-device_interrupts_syscalls.pdf' -> 'Device Interrupts Syscalls'."""
    stem = re.sub(r"^\d+[-_ ]*", "", path.stem)
    words = re.split(r"[-_ ]+", stem)
    return " ".join(w.upper() if w.lower() in ACRONYMS else w.capitalize() for w in words if w)


def make_cover(path: Path, chapters: list[Path]) -> None:
    env = os.environ.get
    title, code, program = env("TITLE", "Course Notes"), env("COURSE_CODE", ""), env("PROGRAM", "")
    year, professor, author, logo = env("YEAR", ""), env("PROFESSOR", ""), env("AUTHOR", ""), env("LOGO", "")

    width, height = A4
    c = canvas.Canvas(str(path), pagesize=A4)
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

    if program:                                  # degree programme above the title
        c.setFont("Helvetica", 13)
        c.drawCentredString(width / 2, y, program)
        y -= 34
    c.setFont("Helvetica-Bold", 24)
    c.drawCentredString(width / 2, y, title)
    y -= 26
    details = " · ".join(x for x in (code, f"A.Y. {year}" if year else "") if x)
    if details:                                  # course code and academic year
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
    y -= 60

    c.setFont("Helvetica-Bold", 13)
    c.drawString(90, y, "Contents")
    y -= 22
    c.setFont("Helvetica", 12)
    for i, ch in enumerate(chapters, 1):
        c.drawString(100, y, f"{i}.  {chapter_title(ch)}")
        y -= 18

    c.setFont("Helvetica", 9)
    c.drawCentredString(width / 2, 50, f"Generated on {datetime.date.today():%d/%m/%Y}")
    c.showPage()
    c.save()


def main() -> None:
    notes = Path(os.environ.get("NOTES_DIR", "notes"))
    output = Path(os.environ.get("OUTPUT", notes / "merged" / "notes.pdf"))
    chapters = sorted(p for p in notes.glob("*.pdf") if p.resolve() != output.resolve())
    if not chapters:
        sys.exit(f"no PDF found in {notes}")

    output.parent.mkdir(parents=True, exist_ok=True)
    cover = output.with_name("_cover.pdf")
    make_cover(cover, chapters)

    writer = PdfWriter()
    writer.append(str(cover))
    for ch in chapters:
        page = len(writer.pages)
        writer.append(str(ch))
        writer.add_outline_item(chapter_title(ch), page)   # bookmark at the chapter start
    with open(output, "wb") as f:
        writer.write(f)
    cover.unlink()
    print(f"{output}: cover + {len(chapters)} chapters, {len(writer.pages)} pages")


if __name__ == "__main__":
    main()
