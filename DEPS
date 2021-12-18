vars = {
    'chromium_git': 'https://chromium.googlesource.com',
    'buildtools_revision': 'd7bdd6f0386aaf20cd00a01d499e8ce1cbf6063e',
}

deps = {
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
