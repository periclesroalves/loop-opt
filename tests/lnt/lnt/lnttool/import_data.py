import os, pprint, sys, time

import lnt.formats
import lnt.util.ImportData
import lnt.server.instance

def action_import(name, args):
    """import test data into a database"""

    from optparse import OptionParser, OptionGroup

    parser = OptionParser("%s [options] <instance path> <file>+"%name)
    parser.add_option("", "--database", dest="database", default="default",
                      help="database to write to [%default]")
    parser.add_option("", "--format", dest="format",
                      choices=lnt.formats.format_names + ['<auto>'],
                      default='<auto>')
    parser.add_option("", "--commit", dest="commit", type=int,
                      default=False)
    parser.add_option("", "--show-sql", dest="show_sql", action="store_true",
                      default=False)
    parser.add_option("", "--show-sample-count", dest="show_sample_count",
                      action="store_true", default=False)
    parser.add_option("", "--show-raw-result", dest="show_raw_result",
                      action="store_true", default=False)
    parser.add_option("-v", "--verbose", dest="verbose",
                      help="show verbose test results",
                      action="store_true", default=False)
    parser.add_option("-q", "--quiet", dest="quiet",
                      help="don't show test results",
                      action="store_true", default=False)
    parser.add_option("", "--no-email", dest="no_email",
                      action="store_true", default=False)
    parser.add_option("", "--no-report", dest="no_report",
                      action="store_true", default=False)
    (opts, args) = parser.parse_args(args)

    if len(args) < 2:
        parser.error("invalid number of arguments")

    path = args.pop(0)

    # Load the LNT instance.
    instance = lnt.server.instance.Instance.frompath(path)
    config = instance.config

    # Get the database.
    db = config.get_database(opts.database, echo=opts.show_sql)

    # Load the database.
    success = True
    for file in args:
        result = lnt.util.ImportData.import_and_report(
            config, opts.database, db, file,
            opts.format, opts.commit, opts.show_sample_count,
            opts.no_email, opts.no_report)

        success &= result.get('success', False)
        if opts.quiet:
            continue

        if opts.show_raw_result:
            pprint.pprint(result)
        else:
            lnt.util.ImportData.print_report_result(result, sys.stdout,
                                                    sys.stderr,
                                                    opts.verbose)

    if not success:
        raise SystemExit, 1

