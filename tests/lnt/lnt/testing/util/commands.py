"""
Miscellaneous utilities for running "scripts".
"""

import errno
import inspect
import os
import sys

# FIXME: Find a better place for this code.

def _write_message(kind, message):
    # Get the file/line where this message was generated.
    f = inspect.currentframe()
    # Step out of _write_message, and then out of wrapper.
    f = f.f_back.f_back
    file,line,_,_,_ = inspect.getframeinfo(f)
    location = '%s:%d' % (os.path.basename(file), line)

    print >>sys.stderr, '%s: %s: %s' % (location, kind, message)

note = lambda message: _write_message('note', message)
warning = lambda message: _write_message('warning', message)
error = lambda message: _write_message('error', message)
fatal = lambda message: (_write_message('fatal error', message), sys.exit(1))

def rm_f(path):
    try:
        os.remove(path)
    except OSError,e:
        if e.errno != errno.ENOENT:
            raise

def mkdir_p(path):
    """mkdir_p(path) - Make the "path" directory, if it does not exist; this
    will also make directories for any missing parent directories."""
    import errno

    try:
        os.makedirs(path)
    except OSError as e:
        # Ignore EEXIST, which may occur during a race condition.        
        if e.errno != errno.EEXIST:
            raise

def capture_with_result(args, include_stderr=False):
    import subprocess
    """capture_with_result(command) -> (output, exit code)

    Run the given command (or argv list) in a shell and return the standard
    output and exit code."""
    stderr = subprocess.PIPE
    if include_stderr:
        stderr = subprocess.STDOUT
    try:
        p = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=stderr)
    except OSError,e:
        if e.errno == errno.ENOENT:
            fatal('no such file or directory: %r when running %s.' % (args[0], \
                                                                     ' '.join(args)))
        raise
    out,_ = p.communicate()
    return out,p.wait()

def capture(args, include_stderr=False):
    import subprocess
    """capture(command) - Run the given command (or argv list) in a shell and
    return the standard output."""
    return capture_with_result(args, include_stderr)[0]

def which(command, paths = None):
    """which(command, [paths]) - Look up the given command in the paths string
    (or the PATH environment variable, if unspecified)."""

    if paths is None:
        paths = os.environ.get('PATH','')

    # Check for absolute match first.
    if os.path.exists(command):
        return command

    # Would be nice if Python had a lib function for this.
    if not paths:
        paths = os.defpath

    # Get suffixes to search.
    pathext = os.environ.get('PATHEXT', '').split(os.pathsep)

    # Search the paths...
    for path in paths.split(os.pathsep):
        for ext in pathext:
            p = os.path.join(path, command + ext)
            if os.path.exists(p):
                return p

    return None

def resolve_command_path(name):
    """Try to make the name/path given into an absolute path to an
    executable.

    """
    # If the given name exists (or is a path), make it absolute.
    if os.path.exists(name):
        return os.path.abspath(name)

    # Otherwise we most likely have a command name, try to look it up.
    path = which(name)
    if path is not None:
        note("resolved command %r to path %r" % (name, path))
        return path

    # If that failed just return the original name.
    return name

def isexecfile(path):
    """Does this path point to a valid executable?

    """
    return os.path.isfile(path) and os.access(path, os.X_OK)
