# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

# def options(opt):
#     pass

def configure(conf):              
    # Check if SQLite is being included
    have_sqlite3 = conf.check_cfg(package='sqlite3', uselib_store='SQLITE3',
                                  args=['--cflags', '--libs'],
                                  mandatory=False)
                                                
    # Only enable ORAN if SQLite is included
    conf.env['ORAN'] = have_sqlite3
    if conf.env['ORAN']:
      conf.env['ENABLE_ORAN'] = True
    else:
      conf.env['MODULES_NOT_BUILT'].append('oran')

    # Explain why ORAN is not being built
    conf.report_optional_feature("ORAN", "ORAN models",
                                 conf.env['ORAN'],
                                 "library 'sqlite3' not found")

def build(bld):
    # Don't do anything if it is determined ORAN shouldn't be built
    if 'oran' in bld.env['MODULES_NOT_BUILT']:
       return

    module = bld.create_ns3_module('oran', ['core', 'network', 'lte'])
    module.source = [
        'model/oran-near-rt-ric.cc',
        'model/oran-lm.cc',
        'model/oran-lm-noop.cc',
        'model/oran-lm-lte-2-lte-distance-handover.cc',
        'model/oran-lm-lte-2-lte-ml-handover.cc',
        'model/oran-cmm.cc',
        'model/oran-cmm-noop.cc',
        'model/oran-cmm-handover.cc',
        'model/oran-cmm-single-command-per-node.cc',
        'model/oran-command.cc',
        'model/oran-command-lte-2-lte-handover.cc',
        'model/oran-report.cc',
        'model/oran-report-apploss.cc',
        'model/oran-report-location.cc',
        'model/oran-report-lte-ue-cell-info.cc',
        'model/oran-reporter.cc',
        'model/oran-reporter-apploss.cc',
        'model/oran-reporter-location.cc',
        'model/oran-reporter-lte-ue-cell-info.cc',
        'model/oran-data-repository.cc',
        'model/oran-data-repository-sqlite.cc',
        'model/oran-near-rt-ric-e2terminator.cc',
        'model/oran-e2-node-terminator.cc',
        'model/oran-e2-node-terminator-wired.cc',
        'model/oran-e2-node-terminator-lte-enb.cc',
        'model/oran-e2-node-terminator-lte-ue.cc',
        'model/oran-e2-node-terminator-container.cc',
        'helper/oran-helper.cc',
        'model/oran-report-trigger.cc',
        'model/oran-report-trigger-periodic.cc',
        'model/oran-report-trigger-lte-ue-handover.cc',
        'model/oran-report-trigger-location-change.cc',
        'model/oran-query-trigger.cc',
        'model/oran-query-trigger-noop.cc',
        'model/oran-query-trigger-custom.cc',
        'model/psc-video-streaming.cc',
        'model/psc-video-streaming-distributions.cc',
        ]

    module_test = bld.create_ns3_module_test_library('oran')
    module_test.source = [
        'test/oran-test-suite.cc',
        ]
    # Tests encapsulating example programs should be listed here
    if (bld.env['ENABLE_EXAMPLES']):
        module_test.source.extend([
        #'test/oran-examples-test-suite.cc',
             ])

    headers = bld(features='ns3header')
    headers.module = 'oran'
    headers.source = [
        'model/oran-near-rt-ric.h',
        'model/oran-lm.h',
        'model/oran-lm-noop.h',
        'model/oran-lm-lte-2-lte-distance-handover.h',
        'model/oran-lm-lte-2-lte-ml-handover.h',
        'model/oran-cmm.h',
        'model/oran-cmm-handover.h',
        'model/oran-cmm-noop.h',
        'model/oran-cmm-single-command-per-node.h',
        'model/oran-command.h',
        'model/oran-command-lte-2-lte-handover.h',
        'model/oran-report.h',
        'model/oran-report-apploss.h',
        'model/oran-report-location.h',
        'model/oran-report-lte-ue-cell-info.h',
        'model/oran-reporter.h',
        'model/oran-reporter-apploss.h',
        'model/oran-reporter-location.h',
        'model/oran-reporter-lte-ue-cell-info.h',
        'model/oran-data-repository.h',
        'model/oran-data-repository-sqlite.h',
        'model/oran-near-rt-ric-e2terminator.h',
        'model/oran-e2-node-terminator.h',
        'model/oran-e2-node-terminator-wired.h',
        'model/oran-e2-node-terminator-lte-enb.h',
        'model/oran-e2-node-terminator-lte-ue.h',
        'model/oran-e2-node-terminator-container.h',
        'helper/oran-helper.h',
        'model/oran-report-trigger.h',
        'model/oran-report-trigger-periodic.h',
        'model/oran-report-trigger-lte-ue-handover.h',
        'model/oran-report-trigger-location-change.h',
        'model/oran-query-trigger.h',
        'model/oran-query-trigger-custom.h',
        'model/psc-video-streaming-distributions.h',
        'model/psc-video-streaming.h',
        ]

    if bld.env.ENABLE_EXAMPLES:
        bld.recurse('examples')

    bld.ns3_python_bindings()

