# 🐛 Compte rendu de debug — Codexion (Scheduler FIFO/EDF)

## 📋 Résumé

| | |
|---|---|
| **Projet** | Codexion (42) |
| **Zone touchée** | `scheduler_dispatch.c`, `dongle.c` |
| **Symptôme** | Un coder attend inutilement alors que ses dongles sont libres |
| **Cause** | Verrou bloquant utilisé pour un simple test de disponibilité |
| **Fix** | `pthread_mutex_lock()` → `pthread_mutex_trylock()` |
| **Statut** | ✅ Corrigé et vérifié par plusieurs runs |

---

## 1. Le problème observé

### Ce qu'on attendait

Deux coders dont les **dongles ne se chevauchent pas** (ex : coder 0 utilise {0,1}, coder 2 utilise {2,3}) doivent pouvoir compiler **en parallèle**, sans se bloquer.

### Ce qu'on a observé

```
0   1 has taken a dongle
0   1 is compiling
200 1 is debugging
200 3 has taken a dongle   ← devrait démarrer à t=0, pas t=200
200 3 is compiling
```

Le coder aux dongles {2,3} attendait systématiquement la fin du compile d'un autre coder — **alors qu'il n'avait besoin d'aucun dongle occupé**.

### Pourquoi c'est grave

- **FIFO** : une requête libre est retardée sans raison → arbitrage injuste.
- **EDF** : une requête avec la **deadline la plus urgente** peut se retrouver bloquée derrière une requête non liée → risque de burnout évitable (violation de la garantie de liveness exigée par le sujet).

---

## 2. Diagnostic

### Commandes utilisées pour investiguer

```sh
make fclean && make tsan          # build avec ThreadSanitizer
./codexion_tsan 4 1000 200 200 200 3 100 fifo 2>&1 | sed -r 's/\x1B\[[0-9;]*[mK]//g' >> repro.txt
```

- `make tsan` → détecte les races (aucune trouvée ici, le bug est un problème de **logique**, pas de race).
- `sed -r 's/\x1B\[[0-9;]*[mK]//g'` → supprime les codes couleur ANSI pour un fichier texte lisible.

### Logs de debug temporaires ajoutés

Dans `coder_acquire.c`, fonction `take_dongles()`, pour savoir **quel dongle** chaque coder prend réellement (le format de log officiel ne l'indique pas) :

```c
log_event(sim, context->coder->id, "has taken a dongle");
fprintf(stderr, "[DEBUG] coder %d took dongle %d\n",
    context->coder->id, first);
```

*(retiré une fois le diagnostic confirmé — casse le format de log imposé par le sujet)*

### Cause identifiée

Dans `dongle.c`, `try_reserve_dongle()` — utilisée par le **scheduler** pour vérifier si un dongle est libre :

```c
pthread_mutex_lock(&sim->dongles[idx].mutex);   // ⚠️ BLOQUE si occupé
```

Ce mutex est aussi tenu par le **coder**, dans `coder_acquire.c` → `take_dongles()`, pendant **toute la durée du compile** :

```c
pthread_mutex_lock(&dongles[first].mutex);  // reste verrouillé ~200ms
```

➡️ Résultat : le scheduler, en voulant juste **tester** la disponibilité d'un dongle occupé, se retrouve à **attendre** la fin du compile en cours — et comme `dispatch_pending()` traite les requêtes une par une dans l'ordre, tout ce qui suit dans le même paquet est retardé, y compris des requêtes totalement indépendantes.

---

## 3. La solution

### Principe

| Fonction | Comportement si déjà verrouillé |
|---|---|
| `pthread_mutex_lock()` | **Attend** que le verrou se libère |
| `pthread_mutex_trylock()` | Retourne **immédiatement** un échec, sans attendre |

Un test de disponibilité doit être **instantané** — `trylock` est donc l'outil adapté, pas `lock`.

### Modification appliquée

**Fichier : `dongle.c` — fonction `try_reserve_dongle()`**

```diff
 static int	try_reserve_dongle(t_simulation_data *sim, int idx,
 		long current_time)
 {
-	pthread_mutex_lock(&sim->dongles[idx].mutex);
+	if (pthread_mutex_trylock(&sim->dongles[idx].mutex) != 0)
+		return (0);
 	if (sim->dongles[idx].is_reserved)
 	{
 		pthread_mutex_unlock(&sim->dongles[idx].mutex);
 		return (0);
 	}
 	...
```

Une seule ligne changée. `take_dongles()` (côté coder) n'est **pas modifiée** : son verrou bloquant est légitime, car il représente une vraie **possession** du dongle pendant le compile, pas un simple test.

---

## 4. Vérification

### Commande de test (5 runs répétés)

```sh
for i in 1 2 3 4 5; do
  echo "=== RUN $i ===" >> repro.txt
  ./codexion_tsan 4 1000 200 200 200 3 100 fifo 2>&1 | sed -r 's/\x1B\[[0-9;]*[mK]//g' >> repro.txt
done
```

### Résultat après correction

```
1 1 has taken a dongle
1 1 is compiling
2 3 has taken a dongle   ← démarre presque au même instant (t=2), plus t=200
2 3 is compiling
```

- ✅ Les deux coders démarrent quasi simultanément (écart de 1 à 2 ms dû à l'ordonnancement OS, normal).
- ✅ Aucun `WARNING: ThreadSanitizer` dans les 5 runs → pas de nouvelle race introduite.
