project('SSL Project', 'c')

openssl_dep = dependency('openssl')

add_project_arguments('-DDEBUG', language: 'c')

executable('server', 'src/server.c', dependencies: openssl_dep)
executable('client', 'src/client.c', dependencies: openssl_dep)