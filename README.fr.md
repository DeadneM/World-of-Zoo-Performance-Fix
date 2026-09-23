<div align="center">

# World of Zoo Performance Fix

**Des menus plus fluides, moins de blocages des tampons graphiques et un mode 60 FPS avec correction du temps physique.**

World of Zoo · Steam · Windows · Direct3D 9 · 32 bits

**[Télécharger la V4](https://github.com/DeadneM/World-of-Zoo-Performance-Fix/releases/tag/v4)** · **[English](README.md)** · [Signaler un problème](https://github.com/DeadneM/World-of-Zoo-Performance-Fix/issues)

</div>

La V4 est la version actuelle, retenue après des comparaisons en jeu à **60 FPS**. Elle corrige certains accès aux tampons graphiques et remplace un pas de temps physique fixe par la durée mesurée de chaque image. Deux fichiers suffisent pour l'installer ; l'exécutable original du jeu reste intact.

> **Pour jouer directement :** télécharger `World-of-Zoo-Performance-Fix-v4.zip`, puis suivre l'installation ci-dessous. Les réglages fournis sélectionnent déjà 60 FPS.

## Sommaire

- **Jouer :** [Installation](#installation) · [Vérifier le fonctionnement](#vérifier-le-fonctionnement) · [Réglages](#réglages)
- **Résoudre un problème :** [Compatibilité](#compatibilité) · [Dépannage](#dépannage) · [Retour à la V2 et désinstallation](#retour-à-la-v2-et-désinstallation)
- **Comprendre :** [Les corrections](#les-corrections) · [Validation](#validation) · [Détails techniques](#détails-techniques) · [Sources et développement](#sources-et-développement)

## Installation

1. **Fermer le jeu.** Télécharger et extraire **[World-of-Zoo-Performance-Fix-v4.zip](https://github.com/DeadneM/World-of-Zoo-Performance-Fix/releases/download/v4/World-of-Zoo-Performance-Fix-v4.zip)** dans la section **Assets** de la release. Les téléchargements automatiques « Source code » de GitHub ne contiennent pas la DLL prête à l'emploi.
2. **Ouvrir le dossier du jeu.** Dans Steam : clic droit sur World of Zoo → **Propriétés → Fichiers installés → Parcourir**. Repérer `WoZRetail.exe`.
3. **Sauvegarder toute `d3d9.dll` existante** en dehors de ce dossier. Un autre mod ou outil graphique peut déjà utiliser ce nom.
4. Ouvrir le dossier extrait **`World-of-Zoo-Performance-Fix-v4`** et **copier ces deux fichiers** à côté de `WoZRetail.exe` :

   ```text
   World of Zoo/
   ├── WoZRetail.exe
   ├── GameLauncher.exe
   ├── d3d9.dll                 ← le correctif
   └── WoZPerformanceFix.ini    ← les réglages
   ```

5. **Lancer normalement le jeu depuis Steam.** Aucun installateur ni modification manuelle de l'exécutable n'est nécessaire.

Les sources, la documentation et le dossier `Retour-V2` ne sont pas nécessaires pour jouer. Les conserver en dehors du dossier du jeu. Un INI de la V3/V4 reste compatible s'il utilise `TargetFPS=60` et `ReadOnlyMeshQueries=1`.

## Vérifier le fonctionnement

Après le lancement, ouvrir **`WoZPerformanceFix.log`** à côté de la DLL et lire la **dernière session**, en bas du fichier. Avec les réglages par défaut, rechercher :

```text
APPLIED V4: target 60 FPS
Physics step uses actual frame delta
Original Windows Direct3DCreate9: OK
```

Les deux premiers extraits figurent sur la même ligne du journal. Les lignes `APPLIED V2` et `APPLIED V3` sont aussi normales : la V4 reprend ces corrections de tampons.

La DLL publiée affiche encore **`V4 experimental`** et **`Gameplay validation required`**. Ces messages historiques ont été conservés pour distribuer exactement le binaire accepté lors des essais. Ils ne signalent pas un autre téléchargement.

## Réglages

Modifier **`WoZPerformanceFix.ini`** à côté de la DLL, enregistrer puis **redémarrer le jeu**.

```ini
[Timing]
TargetFPS=60

[Buffers]
ReadOnlyMeshQueries=1
```

| Réglage | Valeur | Effet |
| --- | --- | --- |
| `TargetFPS` | `60` par défaut | Active le nouveau limiteur et la correction du temps physique associée. **60 FPS est la cible essayée en jeu.** |
| `TargetFPS` | `0` | Conserve le limiteur et le pas physique natifs, autour de 30 FPS. **Zéro ne signifie pas illimité.** |
| `ReadOnlyMeshQueries` | `1` par défaut | Active les accès en lecture seule sur le chemin de requêtes de triangles identifié. |
| `ReadOnlyMeshQueries` | `0` | Rétablit les verrous ordinaires de la V2 ; les autres corrections de tampons restent actives. |

Pour retrouver le temps natif et les verrous de la V2 au sein de la V4, utiliser `TargetFPS=0` et `ReadOnlyMeshQueries=0`. Pour retrouver exactement le binaire V2, voir le [retour à la V2](#retour-à-la-v2-et-désinstallation).

**Au-delà de 60 FPS :** le réglage accepte des nombres entiers de 30 à 360, mais le jeu n'a pas été validé à ces cadences supérieures. Utiliser 60 pour cette version. Les valeurs numériques non prises en charge reviennent à 60. La DLL ne modifie ni la VSync, ni la résolution, ni les options graphiques ; d'autres limites peuvent donc affecter la cadence obtenue.

## Compatibilité

| Élément | Périmètre pris en charge |
| --- | --- |
| Jeu | La **version Steam pour Windows** analysée, `WoZRetail.exe` |
| Architecture | **x86 / 32 bits**, même lorsque le jeu tourne sous Windows 64 bits |
| Graphismes | **Direct3D 9** de Windows |
| Autres éditions | Non validées ; les signatures d'instructions attendues doivent correspondre |
| Autres outils graphiques | Le chargement en chaîne d'une autre `d3d9.dll` n'est pas implémenté |

Le correctif transmet `Direct3DCreate9` à la bibliothèque système de Windows. Il ne charge pas en chaîne ReShade, DXVK, dgVoodoo ou un autre relais local. Conserver une sauvegarde de tout outil existant avant de le remplacer. Aucun exécutable, fichier de données ou sauvegarde du jeu n'est fourni.

## Dépannage

| Symptôme | Vérification à faire |
| --- | --- |
| Aucun journal n'apparaît | Vérifier le nom réel `d3d9.dll`, sa présence à côté de `WoZRetail.exe` et la possibilité pour le jeu d'écrire dans ce dossier. Vérifier que l'archive n'a pas été extraite dans un sous-dossier supplémentaire. |
| Toujours autour de 30 FPS | Vérifier `TargetFPS=60`, redémarrer et chercher la ligne de réussite V4 dans la dernière session du journal. Vérifier aussi la VSync ou un limiteur externe. |
| `NOT APPLIED: executable signatures do not match` | Le code chargé ne correspond pas aux instructions attendues. Comparer l'empreinte de l'exécutable original ci-dessous et vérifier si un autre mod modifie ces instructions. |
| `FPS/PHYSICS MODE NOT APPLIED` | La correction associant limiteur et physique a été refusée. Le temps natif est conservé ; les corrections de tampons installées avec succès peuvent rester actives. Joindre cette ligne au signalement. |
| `READONLY NOT APPLIED` | Le chemin optionnel en lecture seule a été refusé. Les autres lignes du journal indiquent quelles corrections de tampons ont été installées. |
| Saccades, vitesse inhabituelle ou plantage | Comparer la même scène avec `TargetFPS=0`, puis avec la DLL V2 fournie si nécessaire. Indiquer la scène, les réglages et les lignes utiles du journal. |
| Le journal mentionne V2, V3 ou « experimental » | C'est attendu avec la DLL V4 de référence ; voir [Vérifier le fonctionnement](#vérifier-le-fonctionnement). |

Pour [signaler un problème](https://github.com/DeadneM/World-of-Zoo-Performance-Fix/issues), préciser la version du correctif, celle de Windows, la carte graphique et son pilote, l'empreinte de l'exécutable du jeu, les réglages INI, la scène et les étapes permettant de reproduire le problème. Préciser si la V2 se comporte différemment. Les lignes utiles du journal suffisent ; ne pas joindre l'exécutable du jeu ni une copie de sa mémoire.

## Retour à la V2 et désinstallation

**Revenir à la V2 :** fermer le jeu, puis remplacer la DLL installée par **`Retour-V2/d3d9.dll`** dans l'archive V4. C'est exactement le binaire V2 publié. La V2 ignore l'INI de la V4. La [release V2 séparée](https://github.com/DeadneM/World-of-Zoo-Performance-Fix/releases/tag/v2) reste disponible.

**Retirer le correctif :** fermer le jeu et supprimer la `d3d9.dll` fournie par ce projet. Il est également possible de supprimer ses fichiers `WoZPerformanceFix.ini` et `WoZPerformanceFix.log`. Remettre toute ancienne `d3d9.dll` sauvegardée. Aucune restauration de l'exécutable n'est nécessaire : le correctif agit uniquement en mémoire pendant l'exécution du jeu.

## Les corrections

| Partie du jeu | Problème ciblé | Comportement de la V4 |
| --- | --- | --- |
| Menus | Le remplacement complet d'un tampon d'indices de l'interface utilisait un verrou ordinaire susceptible d'attendre le GPU. | Utilise DISCARD pour l'envoi dynamique de l'interface dont le chemin a été vérifié. |
| Maillages animés et requêtes CPU | Le processeur doit lire et conserver les données des maillages, ce qui rend certains accès aux tampons coûteux. | Utilise le chemin de tampons gérés et lisibles du moteur pour la classe de maillages identifiée. |
| Requêtes de triangles | Un chemin de lecture vérifié utilisait des verrous de lecture/écriture ordinaires. | Demande READONLY seulement si les deux contrôles d'appelants et les propriétés des tampons correspondent. |
| Limite d'images | La boucle principale attendait environ 33 ms par itération. | Utilise un limiteur précis réglé à 60 FPS par défaut. |
| Temps physique | Le chemin physique identifié recevait encore 1/30 de seconde à chaque mise à jour à 60 FPS. | Transmet la durée mesurée de l'image à ce chemin. |

Ces corrections sont ciblées. Passer tous les tampons en DISCARD ferait perdre des données nécessaires à certaines fonctions du jeu. Augmenter les FPS ne corrige pas non plus automatiquement tous les compteurs qui avancent d'une valeur fixe par image.

## Validation

| Vérification | Résultat et portée |
| --- | --- |
| Captures locales en jeu | Généralement **59–60 FPS** avec la V4, contre environ **30 FPS** avec la V2. Version acceptée après cette comparaison. |
| Test des instructions physiques natives | Pour 600 mises à jour sur 10 secondes réelles, l'ancien chemin transmet environ **20 secondes** à la fonction physique ; le chemin corrigé en transmet **10**. |
| Vérifications automatisées | Contrôles des tampons, instructions natives, limiteur, préservation des registres et états, chemins d'échec et transmission à Direct3D Windows réussis. |
| Initialisation complète | Neuf cas couvrent les réglages par défaut et natifs, les cibles invalides, les signatures différentes et le refus de la seconde écriture du correctif temporel. |

Le test physique remplace l'intégration finale du monde par une fonction de test ; il ne simule pas une partie complète. Les deux captures contiennent des actions différentes et ne mesurent donc pas précisément chaque animation. Les résultats dépendent encore du matériel, des pilotes et de la complexité de la scène. Le test graphique optionnel n'a pas pu fonctionner dans l'environnement de développement initial restreint ; aucun résultat de benchmark GPU n'en est déduit.

## Détails techniques

La V4 est une petite DLL relais Direct3D 9 qui applique des modifications x86 vérifiées, relatives à l'adresse du module, lors du premier appel à `Direct3DCreate9`. Les objets Direct3D et leurs tables de fonctions restent ceux de Windows. Les deux modifications temporelles sont installées ensemble : elles sont vérifiées et rendues accessibles en écriture avant de modifier l'un ou l'autre emplacement.

- **Tampons :** DISCARD ciblé pour l'interface, maillages MANAGED lisibles et requêtes de triangles READONLY protégées par des contrôles.
- **Cadence :** échéances calculées avec `QueryPerformanceCounter`, minuterie haute résolution lorsqu'elle est disponible et courte attente finale. Une image en retard réinitialise l'échéancier sans rafale de rattrapage. Aucune modification globale de la résolution des minuteries n'est demandée.
- **Physique :** copie du temps natif mesuré dans `[controller+0x20]` avant son utilisation par le chemin physique existant. L'horloge et sa limite native de 100 ms restent en place.
- **Contrôles des instructions :** 19 signatures ont été vérifiées sur l'image analysée. À l'exécution, les contrôles conditionnent les groupes de corrections correspondants ; la DLL ne calcule pas le SHA-256 de tout l'exécutable au lancement.

Les [notes d'implémentation](docs/TECHNICAL.md) détaillent les adresses, les signatures et le temps physique. L'[analyse des tampons](docs/TECHNICAL-V2.md) conserve les observations à l'origine des changements de la V2. Ces deux documents sont en anglais.

<details>
<summary><strong>Fichiers de référence et empreintes SHA-256</strong></summary>

Ces empreintes identifient le jeu analysé et les fichiers exacts publiés. Une DLL recompilée peut avoir des métadonnées et une empreinte différentes.

| Fichier | Octets |
| --- | ---: |
| `WoZRetail.exe` original pris en charge | 7 405 568 |
| `d3d9.dll` V4 | 218 624 |
| Archive ZIP V4 | 248 142 |
| `d3d9.dll` du retour V2 | 213 504 |

```text
WoZRetail.exe — exécutable Steam original pris en charge
622cd4914c4f85f8af949100746078be6a6c111303758cc74206f210e813bfc3

d3d9.dll — V4
171d0c13914635db7f3d16088399ad22888090a5e0d340c0738c9ef8d4558fc5

World-of-Zoo-Performance-Fix-v4.zip
b843a98304021771914cb1d07bf6a763f6722380d07addf2a10bb2a5af62a9f5

Retour-V2/d3d9.dll
9bb09d0ca77b6259d8785323bdadfe556d925602837ec4a7439bd9e1c9eaf565
```

Depuis le dossier concerné, PowerShell permet de vérifier un fichier avec :

```powershell
Get-FileHash -Algorithm SHA256 .\d3d9.dll
```

Le [fichier de vérification de la release](https://github.com/DeadneM/World-of-Zoo-Performance-Fix/releases/download/v4/SHA256SUMS.txt) identifie le ZIP. L'archive contient aussi les empreintes de ses fichiers.

</details>

## Sources et développement

| Chemin | Rôle |
| --- | --- |
| [`src/woz_fix.c`](src/woz_fix.c) | Initialisation, configuration, contrôle de l'image et corrections de tampons héritées |
| [`src/woz_buffer_pacing.h`](src/woz_buffer_pacing.h) | Requêtes de maillages contrôlées et fonctions du limiteur |
| [`src/woz_physics.h`](src/woz_physics.h) | Pas physique et installation conjointe des modifications temporelles |
| [`src/woz_fix.def`](src/woz_fix.def) | Export de `Direct3DCreate9` |
| [`tests/`](tests/) | Tests des tampons, de la physique, de la cadence, de l'initialisation et de Direct3D |
| [`BUILDING.md`](BUILDING.md) | Commandes de compilation Windows x86, tests et prérequis de l'image d'analyse, en anglais |
| [`CHANGELOG.md`](CHANGELOG.md) · [`version.json`](version.json) | Historique et métadonnées de version |

Les binaires prêts à utiliser se trouvent dans **[Releases](https://github.com/DeadneM/World-of-Zoo-Performance-Fix/releases)**. La branche principale contient les sources et la documentation maintenues ; les tags et archives conservent leur état au moment de la publication. Le nettoyage du dépôt ne remplace pas la DLL V4 validée.
