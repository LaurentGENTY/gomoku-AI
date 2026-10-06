# Gomoku AI — portage WASM + UI web + ressources vitrine

- **Date :** 2026-10-06
- **Statut :** design validé en conversation, en attente de relecture de la spec
- **Repo :** `LaurentGENTY/gomoku-AI` (projet S6 ENSEIRB-MATMECA — E. Duchemin, L. Genty, J. Miens, T. Pemeja)

## 1. Intention

Relancer le projet d'école comme side project vitrine.

- **Livrable obligatoire :** des GIF/vidéos courts du jeu et de l'IA en action, intégrés au README et réutilisables sur le portfolio (`~/perso/portfolio`).
- **Bonus :** le jeu jouable dans le navigateur (GitHub Pages).
- **Règle directrice :** la technologie d'origine reste le cœur. Les règles et l'IA restent en **C**, compilées en WebAssembly avec Emscripten. TypeScript se limite à l'UI et à la glue (orchestration des tours, affichage). Aucune réécriture de logique de jeu ou d'IA en TS.
- **Stack** alignée sur `~/perso/optimized-gameoflife` : Emscripten, Vite + TypeScript sans framework, GitHub Pages.

### Hypothèses
- Le code C d'origine doit rester reconnaissable : on porte, on ne réécrit pas.
- Le code serveur d'origine est perdu (décision B) : il est reconstruit.

## 2. Constats de départ

1. **`src/server/` n'a jamais été commité** (absent de tout l'historique git). Manquent : `move.h`, `board.h`, `bitboard.c/h`, `moves.c/h`, `game.c/h`, `server.c`, `players.c`. Les joueurs 4.2/4.3/4.4 dépendent de `board__explore_line`, non documentée dans le rapport mais couverte par ~20 assertions dans `src/test/test_bitboard.c`.
2. Bitboard en `__uint128_t` (deux entiers, un par couleur) → plateau 11×11 max. Supporté par clang/Emscripten en wasm32.
3. `player4.3.c`, `player4.4.c` et `montecarlo.c` utilisent `pthread`. GitHub Pages ne permet pas les en-têtes COOP/COEP nécessaires aux threads WASM.
4. Chaque joueur expose les mêmes symboles globaux (`play`, `initialize`, `self`, `heuristic_*`) → impossible de lier plusieurs joueurs dans un même module.
5. `struct heuristic` (heuristic3) embarque une pile `H_STACK` de 1 000 000 coups (~8 Mo en wasm32).
6. Le repo contient des binaires et artefacts commités (`build_Debug/`, `test_*`, `gmon.out`, `install/`).

## 3. Décisions

| Sujet | Décision |
|---|---|
| Code serveur | Reconstruit d'après le rapport, les tests existants et l'usage dans les joueurs |
| Chemin vers les GIF | WASM + UI web, GIF enregistrés via Playwright |
| IA exposées | 3 niveaux : Facile = `player4` (4.1), Moyen = `player4.4` profondeur 2, Difficile = `player4.4` profondeur 4 |
| UI | Essentielle, pensée pour les GIF (voir §6) |
| Intégration | Un module WASM par rôle (arbitre, joueur 4.1, joueur 4.4), une IA par Web Worker |
| Emplacement | `web/` dans ce repo, déployé sur GitHub Pages |

### Approches écartées
- **Module monolithique :** collisions de symboles entre joueurs → renommage par macros fragile ; un seul Worker sérialise IA vs IA.
- **Serveur C d'origine + `dlopen` (MAIN_MODULE/SIDE_MODULE) :** le plus fidèle, mais le dynamic linking WASM est lourd, gonfle les binaires et se débogue mal ; risque de coût caché élevé.

## 4. Architecture

```
src/server/            reconstruit (C99) : move.h, board.h, bitboard.c/h, moves.c/h, game.c/h
src/players/           inchangé, sauf les retouches du §5.2
src/wasm/              adaptateurs C fins (exports Emscripten)
  referee_api.c        -> referee.wasm   (bitboard : validité, victoire, plateau plein)
  player_api.c         -> player41.wasm  (player4.c + heuristic0 + matrix + list)
                       -> player44.wasm  (player4.4.c + heuristic3 + matrix + list + bitboard)
web/                   Vite + TS sans framework
  src/game/            match.ts (machine à états), referee.ts (wrapper WASM)
  src/ai/              client.ts, ai.worker.ts (1 worker = 1 instance de joueur)
  src/ui/              board.ts (Canvas), controls.ts
  scripts/record-gifs.ts
docs/media/            GIF + MP4 générés
Makefile               modernisé : cibles natives (tests) + cible `wasm` (emcc)
```

Principes :
1. Le C est la source de vérité pour les règles (`referee.wasm`) et l'IA.
2. Le même C compile en natif et en WASM ; les tests natifs valident la reconstruction avant tout portage.
3. Une IA = un Worker = une instance de module → globals C isolées ; IA vs IA = deux Workers.
4. Hygiène : retrait des binaires/artefacts commités, `.gitignore` adapté.

## 5. Partie C

### 5.1 Reconstruction de `src/server/`
Uniquement ce dont les joueurs et les tests ont besoin.

1. **`move.h`** : `struct move_t { size_t row; size_t col; }`, `enum color_t { BLACK = 0, WHITE = 1 }` (le code fait `(color+1)%2`), `struct col_move_t { struct move_t m; enum color_t c; }`.
2. **`board.h` / `bitboard.h` / `bitboard.c`** : `struct board` avec `__uint128_t* b_w`, `__uint128_t* b_b`, `capacity` (= taille²) et la taille ; bits hors plateau initialisés à 1 (cf. rapport §BitBoard et `test_board__initialize`). Fonctions : `board__initialize`, `board__free`, `board__copy`, `board__add_move`, `board__remove_move`, `board__is_valid_move`, `board__is_full`, `board__won`, `board__select_bit`, `board__get_color`, `board__explore_line`, ainsi que `board__possible_move(s)` si les tests les exigent.
3. **`board__explore_line(board, i, j, direction, color, pattern[9])`** : pour la case (i, j) et une direction parmi 4, incrémente la catégorie de motif formée si `color` jouait en (i, j). Catégories : `FIVE, FOUR_OPEN, FOUR_HALF, FOUR_SPACED, THREE_OPEN, THREE_HALF, TWO_OPEN, TWO_HALF, OTHERS`. La sémantique exacte (indices de direction, case occupée ou non) est déduite des assertions de `test_explore__line` ; en cas d'ambiguïté, les tests font foi.
4. **`moves.c/h`, `game.c/h`** : le minimum pour que `test_moves` et `test_game` compilent et passent.
5. **Hors périmètre :** `server.c`, `players.c` (boucle `dlopen`).

**Critère de réussite :** `make test` passe en natif sur les 5 suites existantes.

### 5.2 Retouches aux joueurs (seules modifications du C d'origine)
1. **Threads** (`player4.4.c`) : macro `GOMOKU_NO_THREADS`. Natif = pthreads conservés ; WASM = évaluation séquentielle des coups racine, une seule copie d'heuristique allouée à la fois (créée, évaluée, libérée).
2. **Profondeur** (`player4.4.c`) : `void set_max_depth(int depth)` plafonne la profondeur fixée dans `play()`. Par défaut 4 (comportement d'origine inchangé). Moyen = 2.
3. **`H_STACK`** : inchangé. Pic mémoire attendu ~16 Mo, compilé avec `-sALLOW_MEMORY_GROWTH=1`.
4. **`printf`** : laissés tels quels (sortie dans la console du Worker).

### 5.3 Adaptateurs `src/wasm/`
Fonctions `EMSCRIPTEN_KEEPALIVE` à types plats (entiers et pointeurs vers tableaux d'entiers).

- **Arbitre :** `ref_new(size)`, `ref_play(row, col, color)` → `0 invalide | 1 ok | 2 gagné | 3 plein`, `ref_free()`.
- **Joueur :** `ai_init(size, color, depth)`, `ai_play(n, rows*, cols*, colors*)` → `row * size + col`, `ai_finalize()`. `depth` est ignoré par 4.1.
- Build : `-sMODULARIZE=1 -sEXPORT_ES6=1`, `-O3`, un `.js` + `.wasm` par module.

## 6. App web

### 6.1 Périmètre UI
- Plateau fixe 10×10 (taille des parties du rapport).
- Choix du niveau (Facile / Moyen / Difficile) et de la couleur humaine (Noir commence).
- Mode IA vs IA avec n'importe quelle paire de niveaux.
- Indicateur « l'IA réfléchit… » + temps du dernier coup ; mise en évidence du dernier coup et de la ligne gagnante ; bouton « Nouvelle partie ».
- Section courte « Comment ça marche » (bitboard 128 bits, minimax alpha-bêta, heuristique des croisements) + lien vers `doc/rapport.pdf`.
- Langue de l'UI : anglais (vitrine publique).

### 6.2 Flux d'un tour
1. Clic → `referee.wasm` (thread principal) → `invalide` (rien), `ok`, `gagné` ou `plein`.
2. Si la partie continue → `postMessage({ type: 'play', moves })` au Worker de l'IA, avec les coups joués depuis son dernier tour (même contrat que `play()` dans `player.h`).
3. UI : indicateur actif, clics bloqués.
4. Réponse `{ type: 'move', row, col, ms }` → validée par l'arbitre. Un coup invalide est une erreur, pas un coup accepté.

IA vs IA : deux Workers, même boucle, délai minimum ~400 ms entre coups.

### 6.3 Modules TS
- `game/match.ts` : machine à états `idle → humanTurn → aiThinking → over`, sans DOM, avec arbitre et IA injectés.
- `game/referee.ts` : wrapper typé de `referee.wasm`.
- `ai/client.ts` + `ai/ai.worker.ts` : protocole de messages typé ; le Worker charge `player41` ou `player44` selon le niveau.
- `ui/board.ts` : rendu Canvas. `ui/controls.ts` : contrôles.

### 6.4 Erreurs
- « Nouvelle partie » pendant un calcul → `worker.terminate()` puis nouveau Worker.
- Échec de chargement WASM → message à la place du plateau.
- Coup invalide de l'IA ou crash du Worker → partie arrêtée, « The AI crashed », détail en console. Pas de reprise automatique.

## 7. Tests et vérification

1. **C natif** : `make test` (5 suites existantes). Valgrind en CI Linux ; optionnel en local (indisponible sur macOS ARM).
2. **Nouveau `test_match` natif** : 4.4 contre un adversaire aléatoire implémenté dans le test lui-même (pas `player1`, pour éviter la collision de symboles du §2.4), sur 10×10, ≥ 9 victoires sur 10.
3. **Nouveau test d'équivalence** : deux binaires distincts (4.4 avec et sans `GOMOKU_NO_THREADS`) impriment leur coup sur 5 positions fixes ; un script compare les sorties, qui doivent être identiques.
4. **WASM (Vitest sous Node)** : l'arbitre détecte une victoire en ligne, colonne et deux diagonales ; chaque IA renvoie un coup valide en < 10 s sur une position de milieu de partie.
5. **TS (Vitest)** : `match.ts` avec doubles — alternance, fin de partie, « Nouvelle partie » pendant `aiThinking`.
6. **Playwright smoke** : chargement, une pierre posée au clic, réponse de l'IA.
7. **CI GitHub Actions** : tests C → emcc (`mymindstorm/setup-emsdk`) → Vitest → build Vite → déploiement Pages sur `master`. `permissions:` minimales, aucun secret.

**Budget de temps :** Difficile < 5 s par coup dans Chrome sur le Mac de Laurent. En cas de dépassement, on réduit `nb_childs` pour Difficile (pas l'algorithme). Le chiffre mesuré est noté dans le README.

## 8. Ressources vitrine

1. `web/scripts/record-gifs.ts` (Playwright, enregistrement vidéo) produit :
   - `human-vs-hard` : partie scriptée (coups humains prédéfinis) contre Difficile ;
   - `ai-vs-ai` : 4.1 contre 4.4 jusqu'à la victoire ;
   - `how-it-thinks` : séquence courte avec indicateur et temps par coup.
2. Conversion `ffmpeg` WebM → GIF (palette optimisée, < 3 Mo chacun) + MP4.
3. Sortie dans `docs/media/`, intégrée au README. Copie vers `~/perso/portfolio` hors périmètre de ce projet.
4. README réécrit en anglais : GIF en tête, lien démo Pages, « How it works », build natif et WASM, crédits des 4 auteurs.

## 9. Jalons

| # | Jalon | Résultat visible | Estimation |
|---|---|---|---|
| 1 | Nettoyage du repo + reconstruction C | `make test` au vert | ~1 j |
| 2 | Retouches joueurs + build WASM | 3 modules + test Node au vert | ~0,5 j |
| 3 | App web | partie jouable en local | ~1 j |
| 4 | CI + Pages + GIF + README | démo en ligne, GIF dans le README | ~0,5 j |

**Repli :** si le jalon 2 bloque (ex. Difficile trop lent malgré la réduction de `nb_childs`), on garde le jalon 1 et on enregistre les GIF depuis un rendu terminal natif. Décision soumise à Laurent avant de basculer.

## 10. Hors périmètre

Mode swap/ouverture, taille de plateau réglable, joueurs 1 / 4.2 / 4.3 / Monte Carlo dans l'UI, annulation de coup, mobile soigné (utilisable mais non optimisé), threads WASM, réécriture du serveur `dlopen`, modification du repo portfolio.
