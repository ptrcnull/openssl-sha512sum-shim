project('openssl-sha512sum-shim', 'c',
  version : '0.1',
  default_options : ['warning_level=3'])

openssl_dep = dependency('openssl', version: '>=3.0')

executable('sha512sum',
           'main.c',
           install : true,
           dependencies: [openssl_dep])
