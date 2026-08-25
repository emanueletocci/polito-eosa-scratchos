# Weekly checkpoint tags

`scratch-os` is a cumulative codebase: the flat, static OS students build
grows week over week. At the end of each week's content, the repository
state is tagged so students (and labs) can reference or reset to any
prior week's ending state.

## Convention

- Tag format: `week-NN-end`, e.g. `week-02-end`, `week-03-end`.
- A tag is created once a week's lecture/coding content is finalized and
  merged — it represents "the code as it stood at the end of week NN,"
  which is exactly the starting point week NN+1's lab (and week NN+2's
  lecture/coding session) build on.
- **No branch protection, no hidden tags.** All tags and full commit
  history remain visible to students at all times (see
  `course-repository-structure` policy) — there is no separate
  "solutions" repo or gated branch.

## Using a checkpoint

```sh
git fetch --tags
git checkout week-04-end   # start from the end of week 4
```

## Week 1

There is no `week-00-end` tag — week 1 starts from an empty/minimal
project scaffold (whatever boot-to-nothing skeleton the first unit
provides), since nothing precedes it.
