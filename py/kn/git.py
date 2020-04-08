import subprocess


def current_branch():
    """Return the current git branch."""
    return subprocess.check_output('git branch --show-current'.split()).decode().strip()


def last_commit_date():
    """Returns the date of the last commit."""
    return subprocess.check_output(['git', 'log', '-1', '--pretty=%ad',
                                    '--date=format:%d %b %H:%M', 'py/kn']).decode().strip()
