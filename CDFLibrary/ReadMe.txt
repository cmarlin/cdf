===============================================================================
    CDFLibrary
===============================================================================

Cascaded distance field encoding:
Encode image to many distance fields images. A distance field is a lattice where we store distance to border in each cell.
Using textures to store a shape this way, linear interpolation (used when texture is magnified) results in a fine look and feel.
If we consider thershold value setting the inside and outside of cluster, once we are outside of the cluster, we can reuse lattice to store 
a new cluster. 
Let's consider the green-white-red italian flag: the distance field used for green storage could also be used for red storage too. So, only
two layers could be used for perfect storage of this image.
This approach is interesting for images composed from areas of same color: vector images or cartoon ones (svg/flash are kind of good support).

Terminology:
* Cluster
* Layer
* Control point

Process:
	1°) Cluster creation (not so easy ;-)) - find coherent group of pixels (high or low res ?)
	2°) find control points from each cluster
	3°) Count conflicts between clusters
	4°) Assign Clusters to Layers, minimising conflicts



/////////////////////////////////////////////////////////////////////////////
Main objectives:

- predictive quality (stability of quality
Utilisable en prod: fiable (testé sur de nombreuses images, jeu de test "officiel ?")
Publication :
- Présentation de la méthode
- screenshots comparatifs sur la qualité, taille mémoire
- performances sur des périphériques mobile/desktop
- impact compression (dxt5-bcn)
Encodage "auto":
- Fixer la qualité, recherche des meilleurs paramètres (résolution/nombre de couches)
- fixer la taille mémoire, recherche du meilleur compromis nombre de couches-résolution

/////////////////////////////////////////////////////////////////////////////
Other objectives:

Combiner plusieurs distance fields pour représenter les angles aigus
Images d'entrée antialiasée, (calculer la distance au bord de la zone sur les pixels "jonction")
gestion du sous échantillonage => moiré lorsque le matériau est vu de loin
mesure de qualité (autre que PSNR?) SSIM / VQM http://www.its.bldrdoc.gov/resources/video-quality-research/software.aspx / http://www2.tkn.tu-berlin.de/research/evalvid/EvalVid/vp8_versus_x264.html
affectation des clusters via une lib externe (coin/?) cf test branch n bound
outil d'encodage avec affichage de l'erreur, recherche auto du meilleur compromis
gestion de la transparence
compatibilité monocouche (fontes)
isolation des clusters avec un algo plus "précis": accepter les dégradés
shader avec antialiasing (contribution suivant la distance)
ex: bande de dégradé(noir-blanc) sur fond gris=> quand la bande va passer par le gris la zone risque d'être fusionnée
outil de démo http://jsfiddle.net/bepa/2QXkp/
autoriser le merge de cluster de même couleur / séparation "1 ctrl point" si la couleur est la même
autoriser le merge de control point s'il sert de séparateur et que la valeur (distance & couleur) sont similaires
support mac/linux (=> iOS), premake ou cmake 

/////////////////////////////////////////////////////////////////////////////
