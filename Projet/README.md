
# Pré-requis :

- Dans la racine, une vidéo nommée `video60fps.mp4`
- Dans la racine, un dossier src comprenant les 2 fichiers `.cpp` et les 2 fichiers `.h` utilisés pour notre algorithme


# Installations éventuelles :

Si vous voulez le lancer via une console (windows/linux) :
```
pip install --upgrade pip
pip install scikit-build
pip install cmake
pip install opencv-python
pip install natsort
```
Ça peut être pip3 plutôt que pip, ça doit dépendre des versions et de ce que vous avez de déjà installé.

Et sous PyCharm :
`Ctrl+Alt+S → "Python Interpreter" → +`
Et vous cherchez/installez "opencv-python" et "natsort"


# Lancement du programme :

`python3 interpolate.py <nb_fps_avant_inter> <nb_fps_apres_inter>`
Exemple :
`python3 interpolate.py 10 30`

Va :
- Créer un dossier results
- Créer un dossier results/video10fps_converted et y placer toutes les images ppm de la version 10 fps de la vidéo
- Créer un dossier results/video10to30fps et y placer toutes les images de la vidéo 10 fps + images interpolées, donc ici image1.ppm, image1a.ppm, image1b.ppm, image2.ppm, etc
- Créer une vidéo results/video10fps_converted.avi (assemblage de la vidéo 10 fps pour pouvoir comparer)
- Créer une vidéo results/video10to30fps.avi qui est donc la vidéo qu'on voulait avoir en sortie

Si vous lancez à partir d'une console Windows/Linux (donc hors IDE), je vous conseille d'ajouter `p` en argument, comme suit :
`python3 interpolate.py 10 30 p`
Pour que les prints soient mieux gérés.
Si vous travaillez sous une IDE ce n'est pas nécessaire (et même déconseillé).


# Notes supplémentaires :

- Les algos C++ sont dans le dossier src. Ils sont compilés automatiquement sur votre machine au premier lancement du programme.

- Les dossiers et vidéos réutilisés sont cleans quand on lance le programme.

- Comme l'algo prend plusieurs secondes à générer chaque image, ça peut être assez long (plusieurs minutes) même pour des rendus 10 à 20 fps.
Pour pallier à ça j'ai ajouté pas mal de print tout au long du programme pour avertir de ce qui se passe.
Les prints se font tous seuls, mais je conseille d'ajouter un argument `p` à la fin de la ligne de commande si vous lancez le programme sur une console windows/linux, c'est pour gérer le fait que ces consoles ne print pas tant qu'un `\n` ne vide pas le buffer. Du coup c'est à utiliser seulement dans ces cas là, pas dans les IDE qui ont généralement une meilleure console.
