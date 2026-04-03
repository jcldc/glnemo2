je suis dans le repertoire de mon projet glnemo2 ecrit en c++, utilise l'api QT6 , la libraiire OPenGL et les shaders glsl.
Ce code fonctionne depuis des années sur differentes versions de Linux.
Mais sous fedora43, qui utilise qt 6.10 , l'application crash avec un core dump.
Depuis vscode et le debugger, j'a pu voir où ce crash survient.
C'est en ligne 536 du fichier qopenglcontext.ccp à l'appel de la fonction Q_D(const QOpenGLContext).
Voici le source :
QOpenGLFunctions *QOpenGLContext::functions() const
{
    Q_D(const QOpenGLContext);
if (!d->functions)
const_cast<QOpenGLFunctions*&>(d->functions) = new QOpenGLExtensions(QOpenGLContext::currentContext());
return d->functions;
}

Il semblerait que le current context est NULL, d'ou le crash. Ca n'arrive que sous fedora43.

Dans le repertoire build se trouve la compilation du projet en mode Debug. Si tu lances depuis ce repertoire bin/glnemo2 le programme va crasher.
Essaie de resoudre le probleme.
