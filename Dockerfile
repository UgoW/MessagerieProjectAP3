# Étape 1 : Utiliser une image Fedora comme base
FROM fedora:38

# Étape 2 : Mettre à jour le système et installer GCC et Make
RUN dnf update -y && \
    dnf install -y gcc make && \
    dnf clean all && \
    rm -rf /var/cache/dnf

# Étape 3 : Définir un répertoire de travail
WORKDIR /app

# Étape 4 : Copier les fichiers dans l'image
COPY . .

# Étape 5 : Compiler les fichiers source
RUN gcc server.c -o server && gcc client.c -o client

# Étape 6 : Définir le conteneur comme prêt à exécuter un programme
CMD ["/bin/bash"]
