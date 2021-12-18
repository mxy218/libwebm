# This file is used to manage the dependencies of the webm/libwebm repo. It is
# used by gclient to determine what version of each dependency to check out, and
# where.
#
# For more information, please refer to the official documentation:
#   https://sites.google.com/a/chromium.org/dev/developers/how-tos/get-the-code
#
# When adding a new dependency, please update the top-level .gitignore file
# to list the dependency's destination directory.
#
# For more on the syntax and semantics of this file, see:
#   https://bit.ly/chromium-gclient-conditionals
#
# which is a bit incomplete but the best documentation we have at the
# moment.

vars = {
    'chromium_git': 'https://chromium.googlesource.com',
    'clang_format_revision': 'e435ad79c17b1888b34df88d6a30a094936e3836',
    'buildtools_revision': 'd7bdd6f0386aaf20cd00a01d499e8ce1cbf6063e',
}

deps = {
    'libwebm/buildtools/clang_format/script':
        Var('chromium_git') +
        '/external/github.com/llvm/llvm-project/clang/tools/clang-format.git@' +
        Var('clang_format_revision'),
    'libwebm/buildtools':
        Var('chromium_git') + '/chromium/src/buildtools.git@' +
        Var('buildtools_revision'),
}

hooks = [
    {
        'name':
            'clang_format_linux',
        'pattern':
            '.',
        'condition':
            'host_os == "linux"',
        'action': [
            'download_from_google_storage.py',
            '--no_resume',
            '--no_auth',
            '--bucket',
            'chromium-clang-format',
            '-s',
            'libwebm/buildtools/linux64/clang-format.sha1',
        ],
    },
]
