# Charte universelle de développement modding

**Version : 1.0 · 10 octobre 2026 · Référence commune des projets DeadneM**

Ce document constitue le socle des projets de modding. Il ne remplace pas le README, le carnet technique ni les instructions particulières à chaque jeu. Le statut *VALIDÉ* relève de l'utilisateur, et non de la compilation ou de l'assistant. Les annexes précisent les exceptions sans contredire les règles de sécurité et de stabilité.

## Les 15 règles impératives

1. **Base stable protégée.** Repartir de la dernière version explicitement validée. Ne jamais écraser ni promouvoir une version expérimentale sans confirmation.
2. **Cumulativité raisonnée.** Préserver toutes les fonctionnalités validées. N'hériter des changements expérimentaux que s'ils sont nécessaires, documentés ou expressément retenus. Éviter le retour des anciens bugs.
3. **Stable et expérimental séparés.** Isoler les essais sur une branche ou une copie de travail. Le nom `main` n'est qu'une convention : respecter l'organisation effective du dépôt.
4. **But mesurable par build.** Préciser problème, hypothèse, modifications, résultat attendu et critère d'acceptation *avant* le test. Une nouvelle découverte n'est pas nécessairement une fonction achevée.
5. **Analyse fondée sur les preuves.** Étudier le code, les fichiers, les logs et les résultats déjà obtenus. Ne pas inférer le succès à partir d'un nom de fonction ou d'une entrée de journal.
6. **Modification proportionnée.** Regrouper les changements liés lorsque cela aide ; isoler les opérations risquées. Prévoir le retour arrière et sauvegarder les données susceptibles d'être altérées.
7. **Statuts explicites.** `CONSTRUIT` = produit ; `CONTRÔLÉ` = structure/intégrité vérifiées ; `EXPÉRIMENTAL` = résultat en jeu incertain ; `VALIDÉ` = confirmé par l'utilisateur ; `REJETÉ` = crash/régression/échec. Ne jamais confondre ces états.
8. **ZIP réellement installable.** Fournir binaires, INI, README et fichiers nécessaires selon le projet, sans dossier englobant superflu. Conserver les véritables sous-dossiers d'installation (`ue4ss/`, `plugins/`, etc.).
9. **Liens fiables.** Privilégier un lien de téléchargement direct durable et, lorsque réalisable, une pièce jointe ZIP. Vérifier que l'archive existe. Ne pas présenter des artefacts GitHub Actions éphémères comme permanents.
10. **GitHub comme sauvegarde.** Conserver sources, builds pertinents, résultats et décisions. Vérifier chaque push annoncé. Un test archivé n'est pas une release stable.
11. **Documentation cumulative.** Actualiser README, historique, architecture, carnet technique et TODO au fil des découvertes. Ne pas supprimer une piste ou un résultat négatif utile.
12. **Compatibilité et configuration.** Préserver raccourcis, valeurs par défaut, INI et comportements validés. Limiter les dépendances, conserver des logs utiles et respecter les contraintes propres au mod (ASI seulement, patch EXE, etc.).
13. **Priorité au résultat utilisateur.** Si plusieurs diagnostics successifs n'apportent aucune progression fonctionnelle, revisiter l'hypothèse ou l'architecture. Ne pas multiplier des builds d'audit quasi identiques.
14. **Autonomie honnête.** Ne pas redemander ce qui est établi. Réexaminer seulement si la version du jeu ou le contexte technique a changé. Signaler franchement les limites, erreurs, incertitudes et actions effectivement effectuées.
15. **Reprise sans perte.** Avant de changer de conversation, identifier dernière base validée, dernier essai, décisions, bugs ouverts, étape suivante et liens de fichiers. Ne pas dépendre exclusivement de la mémoire du chat.

## Ordre de priorité en cas de conflit

1. Sécurité des sauvegardes, compatibilité et stabilité.
2. Préservation des fonctions déjà validées.
3. Objectif réel et exigences explicites du projet.
4. Progression vérifiable et diagnostics utiles.
5. Livraison reproductible et fiable.
6. Documentation et continuité.

Les essais agressifs (limites moteur ×4/×10, hooks, portages, etc.) restent autorisés **sur une base expérimentale isolée et réversible**. Une évolution technique n'est *validée en jeu* qu'après confirmation de l'utilisateur.

## Protocole de livraison

Pour chaque build, indiquer : **base de départ → changements → fonctions préservées → contrôles effectués → statut → procédure de test → lien du ZIP → prochaine étape**. Ne pas déclarer un build, un push ou une release sans l'avoir effectivement réalisé.

Une archive réussie ne prouve pas que le jeu fonctionne. Une application visuelle ne prouve pas un déverrouillage natif, ni un succès réparé.

## Continuité entre conversations

Le dépôt et ses notes sont la source technique de référence. En reprise : lire le README et l'historique, retrouver la base validée et les versions rejetées, puis reprendre *l'objectif fonctionnel*. Signaler une estimation de marge de conversation lorsqu'elle est demandée.

---

## Annexe : World of Zoo

- Préserver le correctif de performances existant et vérifier l'amélioration par des essais reproductibles.
- Maintenir instructions d'installation Steam, README et ressources `docs/images/`.
- Ne pas élargir les changements graphiques sans résultat mesurable et option de retour arrière.
