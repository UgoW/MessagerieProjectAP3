project('SSL Project', 'c')

openssl_dep = dependency('openssl')

executable('server', 'src/server.c', dependencies: openssl_dep)
executable('client', 'src/client.c', dependencies: openssl_dep)
