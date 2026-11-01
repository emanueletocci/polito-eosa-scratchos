# Week 7 laboratory — examination practice on scheduling

Laboratory 7 is practice for the examination. Its goal is that you
solve, unaided, questions of the kind the examination sets: every
exercise is written in the form, and at the length, of an examination
question. It is a bank of seven such questions on week 6's topic, the
scheduling of periodic tasks: the utilisation bound of Rate Monotonic,
response-time analysis, the earliest-due-date rule, and what a kernel
must do so that a task of high priority runs when it should. Each
question states what is required, gives the signatures or the files to
write, and leaves the rest to you. Part A is for the
assisted session; Part B is for work you carry out independently
afterwards. The last question of each part is a composite: it joins
several of the ideas of the course in one program, and is the longest.

## Objectives

On completion you will have:

- computed, in integer C, the utilisation of a task set, the verdict of
  the Liu and Layland test, and the response times of response-time
  analysis;
- scheduled jobs by earliest due date and computed their maximum
  lateness;
- changed the week 5 kernel so that a task of higher priority that
  becomes ready runs at once, and so that a task waiting for input does
  not hold the processor;
- completed the receive interrupt handler of a console and the wait of
  a task for its bytes, from a statement of behaviour alone;
- read a scheduler trace in C, and made the kernel record the deadlines
  its tasks miss, checked against `schedsim`;
- compared each answer with a reference solution, after your own
  attempt.

## Starting point

[`../week-05/solution/`](../week-05/solution/), the finished week 5
operating system, for the questions that change the kernel (Q3, Q4,
Q7): each of them lays its own files over a copy of it, so the folder
itself is never modified. The other questions are C functions that run
alone. Every question is in [`exam-practice/`](exam-practice/); read
[`exam-practice/README.md`](exam-practice/README.md) first: it says
what you write in each question and how to try it.

The kernel questions build on the week 5 kernel with a fixed-priority
choice of the next task, given in their `kernel/task.c`. If you wrote
your own priority scheduler in week 6, you may use it instead of the
given one in a copy of the folder: compare the two first.

## Structure

The questions are independent of each other: none needs the answer of
another, and they can be done in any order, although Part A is ordered
from paper to kernel. Nothing is handed in, and nothing in the folders
checks an answer: the reference solution of each question is in
[`exam-practice/solutions/`](exam-practice/solutions/), to be read after
your own attempt. Deciding whether your code is right, by trying the
edges as well as the examples, is part of each question.

Two conventions hold for every question:

- **State what you expect before you run.** For a function, the value it
  must return on the cases you choose; for the kernel, the lines it must
  print and at which tick. When the result differs, you have learned
  something about your model, not only about your code.
- **A trace is evidence.** The kernel questions are judged on what the
  kernel prints: the switches of the trace, with their ticks, and the
  lines of the tasks. Read them against the statement, tick by tick.

---

## Part A — in the laboratory

### Q1. The utilisation bound

[`exam-practice/q01-ll-bound/`](exam-practice/q01-ll-bound/): the
utilisation of a task set in thousandths, and the verdict of the Liu
and Layland test with the bound of the course.

### Q2. Response-time analysis

[`exam-practice/q02-rta/`](exam-practice/q02-rta/): the interference of
the tasks of higher priority, the response time as a fixed point, and
whether the whole set is schedulable.

### Q3. A higher priority that cannot wait

[`exam-practice/q03-wake-preempt/`](exam-practice/q03-wake-preempt/):
the kernel switches to a task of higher priority at the tick at which it
wakes, not at the end of the running task's time slice.

### Q4. A task that waits for input

[`exam-practice/q04-input-task/`](exam-practice/q04-input-task/), a
composite: the gaps of an interrupt-driven console input, so that the
reading task waits without the processor, runs at the tick its byte
arrives, and no waiting byte is lost.

### Done looks like — Part A

- Q1 and Q2 give the values of their examples and of the edges you
  chose, and you can say why each edge was worth trying.
- In Q3 the trace switches to the woken task at its wake tick, and
  nothing changes when the woken task does not outrank the running one.
- In Q4, a key pressed while A runs is read by B at that tick, A ends at
  tick 20 as if alone, and a full ring keeps the bytes it holds.

---

## Part B — independent practice

### Q5. Earliest due date

[`exam-practice/q05-edd/`](exam-practice/q05-edd/): the next job by
earliest due date, the finish time of a job, and the maximum lateness.

### Q6. Reading a trace

[`exam-practice/q06-trace-read/`](exam-practice/q06-trace-read/), a
composite: from the switch records of a trace, the run time of a task,
the response time of a job, the first deadline miss, and whether the
schedule obeys Rate Monotonic.

### Q7. A deadline monitor

[`exam-practice/q07-deadline-monitor/`](exam-practice/q07-deadline-monitor/),
a composite: the kernel records the deadlines its periodic tasks miss,
and the first one agrees with `schedsim`.

### Done looks like — Part B

- You have written, from the statement alone, the answers of the
  questions you chose, and tried each one on the edges as well as on the
  examples.
- You have read the reference solution of each, after your attempt, and
  can say where it differs from yours and whether that matters.

---

## After the bank

The questions of this bank meet in two places, and the solutions say
where.

- **Analysis and kernel answer the same question.** Q2 and Q6 compute,
  from the task set or from the trace, whether every deadline is met; Q7
  makes the kernel answer it while it runs. For the set of Q7 the three
  agree on the first miss, and where they could disagree, the statement
  of Q7 says why.
- **A handler and a task share data.** In Q4 the receive interrupt
  handler fills the buffer and the system call empties it. The reference
  solution explains why, with the priorities the statement gives, the
  two never interleave, and what could happen with others. Read that
  comment after your attempt.
