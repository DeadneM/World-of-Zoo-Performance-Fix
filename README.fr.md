# World of Zoo Performance Fix — V4

**La V4 est la version retenue après les essais locaux à 60 FPS.** Elle conserve les corrections de tampons de la V2, améliore certaines lectures de maillages et corrige un pas de temps physique resté fixe dans la V3.

[Télécharger la V4](https://github.com/DeadneM/World-of-Zoo-Performance-Fix/releases/tag/v4) · [Version V2 à cadence native](https://github.com/DeadneM/World-of-Zoo-Performance-Fix/releases/tag/v2) · [English](README.md)

## Installation

1. Fermer le jeu et extraire **World-of-Zoo-Performance-Fix-v4.zip**.
2. Ouvrir le dossier contenant `WoZRetail.exe` : Steam → Propriétés → Fichiers installés → Parcourir.
3. Conserver une copie de toute ancienne `d3d9.dll` hors du dossier du jeu.
4. Copier **`d3d9.dll` et `WoZPerformanceFix.ini`**, à la racine de l'archive, à côté de `WoZRetail.exe`.
5. Relancer normalement le jeu par Steam.

La cible est déjà réglée à **60 FPS**. Si la V3 ou la V4 d'essai est installée avec `TargetFPS=60` et `ReadOnlyMeshQueries=1`, l'INI existant reste compatible. La DLL publiée est exactement celle de la V4 essayée en jeu.

## Ce que fait la V4

- Conserve les corrections V2 des tampons de l'interface et des maillages relus par le processeur.
- Utilise des accès en lecture seule pour les requêtes de triangles dont le chemin est vérifié.
- Remplace l'attente native d'environ 33 ms par un limiteur précis à 60 FPS.
- Fournit au chemin physique identifié la durée réelle de l'image au lieu d'une durée fixe de 1/30 de seconde.

La correction physique et le nouveau limiteur sont installés ensemble ; si une vérification échoue, les deux restent d'origine. Le fichier `WoZRetail.exe` sur disque reste intact.

## Validation

Les captures locales montrent généralement 59–60 FPS avec la V4 et environ 30 FPS avec la V2. La V4 a été acceptée pour publication après cette comparaison le 22 septembre 2026.

Les tests sur les instructions du moteur vérifient le pas physique, les tampons, le limiteur, les registres préservés, les refus d'initialisation et la transmission à Direct3D Windows. Le test physique mesure la durée fournie à une fonction de test remplaçant l'intégration finale ; il ne reproduit pas une partie complète.

Les deux captures contiennent des actions différentes : elles ne permettent pas de garantir la même durée pour chaque animation. La correction ne rend pas automatiquement indépendants des FPS tous les compteurs qui ajoutent une valeur fixe par image. Les essais de jeu de cette version concernent **60 FPS**.

## Revenir au mode natif ou à la V2

Pour conserver le limiteur et le pas physique natifs, mettre `TargetFPS=0` dans l'INI puis relancer. Cette option signifie environ 30 FPS, pas une cadence illimitée. `ReadOnlyMeshQueries=0` rétablit également les verrous de la V2.

Pour retrouver exactement la V2 publiée, fermer le jeu et remplacer la DLL par **`Retour-V2/d3d9.dll`**, fourni dans l'archive. La V2 ignore cet INI.

## Journal et compatibilité

La dernière session de `WoZPerformanceFix.log` doit indiquer `APPLIED V4: target 60 FPS` et `Physics step uses actual frame delta`. La DLL conservée porte encore la mention **V4 experimental** dans son journal : c'est bien le binaire accepté après essai.

Version Steam 32 bits analysée, empreinte SHA-256 de `WoZRetail.exe` :

```text
622cd4914c4f85f8af949100746078be6a6c111303758cc74206f210e813bfc3
```

Empreinte de la DLL V4, 218624 octets :

```text
171d0c13914635db7f3d16088399ad22888090a5e0d340c0738c9ef8d4558fc5
```

Les sources et les tests sont fournis. Aucun exécutable du jeu, sauvegarde ou capture privée n'est inclus. [Compilation](BUILDING.md) · [Détails techniques](docs/TECHNICAL.md).
