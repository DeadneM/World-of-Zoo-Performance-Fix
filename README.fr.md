# Correctif de performances pour World of Zoo

Correctif Windows pour **World of Zoo sur Steam, en 32 bits**. Il vise les attentes Direct3D 9 observées dans les menus et lors des accès aux données 3D. **La limite native d'environ 30 FPS est conservée.**

**Version de référence actuelle : V2**, retenue après les essais locaux. Elle inclut la correction des menus de la V1. Le résultat peut varier selon le pilote et la scène ; aucun gain chiffré universel n'est annoncé.

[Télécharger la V2](dist/World-of-Zoo-Performance-Fix-v2.zip) · [English](README.md)

## Installation

1. Fermer le jeu et extraire l'archive ZIP.
2. Ouvrir le dossier contenant `WoZRetail.exe` : Steam → Propriétés → Fichiers installés → Parcourir.
3. Conserver une copie de l'éventuelle `d3d9.dll` actuelle hors du dossier du jeu. Pour notre ancienne V1, la renommer en `d3d9-WoZIndexFix-V1.disabled` convient aussi.
4. Copier **uniquement la nouvelle `d3d9.dll`** à côté de `WoZRetail.exe`.
5. Relancer le jeu depuis Steam.

Une seule DLL locale peut porter le nom `d3d9.dll` : conserver tout autre correctif déjà présent avant de le remplacer.

Dans la dernière session de `WoZIndexFix.log`, vérifier le titre `WoZ Index and Mesh Fix V2`, les deux lignes `APPLIED` concernant les menus et les maillages, puis `Original Windows Direct3DCreate9: OK`.

## Retour en arrière

Fermer le jeu, retirer la DLL ajoutée et restaurer la précédente si nécessaire. Restaurer la sauvegarde V1 sous le nom `d3d9.dll` permet de garder seulement la correction des menus.

Le correctif agit en mémoire au lancement. Il conserve l'EXE sur disque, les sauvegardes, la vitesse de simulation et la limite native de 33 ms.

## Principe et validation

La V1 corrige la réécriture des indices de l'interface. La V2 utilise en plus le stockage géré déjà présent dans le moteur pour les maillages que le processeur relit et modifie. Cela conserve leurs données pour les mises à jour et les calculs sur les triangles.

Neuf signatures du code sont contrôlées. Les tests du code 32 bits et du relais Direct3D passent, avec 10 000 appels du chemin des menus et 10 000 constructions des tampons 3D. Le journal de la V2 installée confirme l'application des deux corrections. Les performances sur d'autres configurations restent à tester.

Le test graphique séparé n'a pas pu créer d'appareil dans l'environnement de développement ; il ne constitue donc pas une mesure de gain. Les détails sont dans [les notes techniques](docs/TECHNICAL.md).

Empreinte SHA-256 de la DLL V2 de référence :

```text
9bb09d0ca77b6259d8785323bdadfe556d925602837ec4a7439bd9e1c9eaf565
```

Les sources, [les instructions de compilation](BUILDING.md) et les tests sont fournis. Aucun exécutable ou fichier de données du jeu n'est inclus.
