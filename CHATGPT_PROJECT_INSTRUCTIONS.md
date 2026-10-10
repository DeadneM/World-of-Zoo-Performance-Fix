# Instructions ChatGPT pour ce projet

Copier le texte suivant dans **••• > Paramètres du projet > Instructions du projet**. Les instructions de projet remplacent les instructions personnalisées globales : inclure donc les règles communes **et** cette annexe.

## Texte à coller

Pour mes projets de modding, applique ces règles :
1. Pars de la dernière base expressément VALIDÉE.
2. Préserve les fonctions validées ; seules les expériences retenues passent à la suite.
3. Sépare branches stables et expérimentales.
4. Définis un objectif et un critère de réussite par build.
5. Étudie d'abord code, README, logs et résultats précédents.
6. Regroupe les changements cohérents, mais isole les risques et prévois rollback.
7. Distingue CONSTRUIT, CONTRÔLÉ, EXPÉRIMENTAL, VALIDÉ par moi, REJETÉ.
8. Livre des ZIP installables, à la racine sans dossier englobant inutile, avec INI/README et arborescence requise.
9. Fournis un lien direct fiable et un ZIP joint si possible.
10. Vérifie réellement les commits, branches, ZIP et releases annoncés.
11. Maintiens README, carnet technique, historique et TODO cumulatifs.
12. Préserve raccourcis, INI, comportements par défaut et compatibilité.
13. Après plusieurs audits sans progrès, revois l'approche ; diagnostic ≠ fonctionnalité.
14. Agis sans reposer les questions réglées, mais n'invente jamais fichier, test, réussite ou capacité.
15. Pour chaque reprise, retrouve base validée, dernière expérimentation, bugs, décisions et prochaine étape.

Priorité : sécurité et sauvegardes > acquis validés > objectif du jeu > progrès vérifiable > livraison > documentation. Après chaque build, indique la base, les changements, ce qui reste intact, le statut, le test à faire, le lien et la suite.

## Annexe : World of Zoo

- Préserver le correctif de performances existant et vérifier l'amélioration par des essais reproductibles.
- Maintenir instructions d'installation Steam, README et ressources `docs/images/`.
- Ne pas élargir les changements graphiques sans résultat mesurable et option de retour arrière.

---
Référence technique complète dans `DEVELOPMENT_RULES.md` du dépôt ; en début de reprise, lire aussi le README et les notes disponibles. Ne pas supposer un accès aux fichiers ou dépôts non effectivement consultés.
