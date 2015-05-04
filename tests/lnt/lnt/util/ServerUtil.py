"""
Utility for submitting files to a web server over HTTP.
"""

import plistlib
import sys
import urllib
import urllib2

import lnt.server.instance
from lnt.util import json
from lnt.util import ImportData

# FIXME: I used to maintain this file in such a way that it could be used
# separate from LNT to do submission. This could be useful for adapting an older
# system to report to LNT, for example. It might be nice to factor the
# simplified submit code into a separate utility.

def submitFileToServer(url, file, commit):
    with open(file, 'rb') as f:
        values = { 'input_data' : f.read(),
                   'commit' : ("0","1")[not not commit] }

    data = urllib.urlencode(values)
    response = urllib2.urlopen(urllib2.Request(url, data))
    result_data = response.read()

    # The result is expected to be a JSON object.
    try:
        return json.loads(result_data)
    except:
        import traceback
        print "Unable to load result, not a valid JSON object."
        print
        print "Traceback:"
        traceback.print_exc()
        print
        print "Result:"
        print result_data
        return

def submitFileToInstance(path, file, commit):
    # Otherwise, assume it is a local url and submit to the default database
    # in the instance.
    instance = lnt.server.instance.Instance.frompath(path)
    config = instance.config
    db_name = 'default'
    db = config.get_database(db_name)
    if db is None:
        raise ValueError("no default database in instance: %r" % (path,))
    return lnt.util.ImportData.import_and_report(
        config, db_name, db, file, format='<auto>', commit=commit)

def submitFile(url, file, commit, verbose):
    # If this is a real url, submit it using urllib.
    if '://' in url:
        result = submitFileToServer(url, file, commit)
        if result is None:
            return
    else:
        result = submitFileToInstance(url, file, commit)
    return result

def submitFiles(url, files, commit, verbose):
    results = []
    for file in files:
        result = submitFile(url, file, commit, verbose)
        results.append(result)
    return results
