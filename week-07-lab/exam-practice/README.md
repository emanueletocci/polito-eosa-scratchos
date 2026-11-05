# Practice questions

Programming questions on the scheduling of periodic tasks, the topic of
week 6, with the kernel of week 5. Each one is a folder,
`qNN-<topic>/`, with a `STATEMENT.md` that says what is required, and
the file you write:

| Question | You write |
|---|---|
| Q1, Q2, Q5, Q6 | `answer.c`, which already holds the types and a stub for every function |
| Q3, Q7 | `kernel/task.c` of the week 5 kernel, next to the other given files of the folder |
| Q4 | the gaps of `kernel/input.c` of the week 5 kernel, marked `gap (a)` to `gap (d)` |

Nothing else in a question folder is needed to answer it. Q4, Q6 and Q7
are composites: they join several parts of the course in one answer.

## Trying your code

`try/` is a convenience for running what you wrote, outside the
questions: nothing in it checks an answer, and deciding whether your code
is right is part of the question. Test the edges, such as a task set with
one task, equal deadlines or periods, and a response time equal to the
period.

From `try/`:

- `make run Q=q02` (Q1, Q2, Q5, Q6) builds your `answer.c` with
  `play/q02.c`, a `main` that includes your file and calls your
  functions, and runs it in QEMU. Edit `play/q02.c` to call the functions
  with the values you choose and print the results with `kprintf`. Stop
  it with `Ctrl-C`. `runtime/` holds what such a program needs: a
  start-up that prepares memory as the boot code of week 2 does, a polled
  console on UART0 with the `kprintf` of week 3, and a linker script with
  the memory map you know. The program runs alone, privileged: there is
  no kernel and no task.
- `make run Q=q03` (Q3, Q4, Q7) builds the kernel of
  `../../week-05/solution/` with the files of the question's `kernel/`
  and `src/` laid over it, and runs it in QEMU, as in week 5. In Q4, what
  you type goes to the console of the running kernel. Change the
  priorities or the periods in the question's `kernel/task.h` to try
  other cases.

## After your attempt

`solutions/qNN-<topic>/` holds a reference solution that meets the
statement: the file you write, with the same name. Read it after your
own attempt, and compare: a solution that differs from yours is not wrong
for that reason alone, so check it against the statement and the cases
you tried.
