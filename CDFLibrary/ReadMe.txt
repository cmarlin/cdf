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
	1�) Cluster creation (not so easy ;-)) - find coherent group of pixels (high or low res ?)
	2�) find control points from each cluster
	3�) Count conflicts between clusters
	4�) Assign Clusters to Layers, minimising conflicts



/////////////////////////////////////////////////////////////////////////////
Main objectives:

- predictive quality (stability of quality
Utilisable en prod: fiable (test� sur de nombreuses images, jeu de test "officiel ?")
Publication :
- Pr�sentation de la m�thode
- screenshots comparatifs sur la qualit�, taille m�moire
- performances sur des p�riph�riques mobile/desktop
- impact compression (dxt5-bcn)
Encodage "auto":
- Fixer la qualit�, recherche des meilleurs param�tres (r�solution/nombre de couches)
- fixer la taille m�moire, recherche du meilleur compromis nombre de couches-r�solution

/////////////////////////////////////////////////////////////////////////////
Other objectives:

Combiner plusieurs distance fields pour repr�senter les angles aigus
Images d'entr�e antialias�e, (calculer la distance au bord de la zone sur les pixels "jonction")
gestion du sous �chantillonage => moir� lorsque le mat�riau est vu de loin
mesure de qualit� (autre que PSNR?) SSIM / VQM http://www.its.bldrdoc.gov/resources/video-quality-research/software.aspx / http://www2.tkn.tu-berlin.de/research/evalvid/EvalVid/vp8_versus_x264.html
affectation des clusters via une lib externe (coin/?) cf test branch n bound
outil d'encodage avec affichage de l'erreur, recherche auto du meilleur compromis
gestion de la transparence
compatibilit� monocouche (fontes)
isolation des clusters avec un algo plus "pr�cis": accepter les d�grad�s
shader avec antialiasing (contribution suivant la distance)
ex: bande de d�grad�(noir-blanc) sur fond gris=> quand la bande va passer par le gris la zone risque d'�tre fusionn�e
outil de d�mo http://jsfiddle.net/bepa/2QXkp/
autoriser le merge de cluster de m�me couleur / s�paration "1 ctrl point" si la couleur est la m�me
autoriser le merge de control point s'il sert de s�parateur et que la valeur (distance & couleur) sont similaires
support mac/linux (=> iOS), premake ou cmake 

/////////////////////////////////////////////////////////////////////////////
