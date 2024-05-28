# How to contribute to shapelib

# Commit hooks

shapelib provides pre-commit hooks to run code linters before a commit is made. The
hooks are cloned with the repository and can be installed using
[pre-commit](https://pre-commit.com>):

```shell
    python -m pip install pre-commit
    pre-commit install
```

Once installed, the hooks can be run manually via ``pre-commit run --all-files``.

# Blame ignore file

Due to whole-tree code reformatting, ``git blame`` information might be
misleading. To avoid that, you need to modify your git configuration as following
to ignore the revision of the whole-tree reformatting:

```shell
    git config blame.ignoreRevsFile .git-blame-ignore-revs
```
