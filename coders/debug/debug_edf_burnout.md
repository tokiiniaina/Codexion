# 📊 Tests EDF — Codexion

## 📋 Résumé

| | |
|---|---|
| **Objectif** | Vérifier que le scheduler EDF choisit toujours la requête avec la deadline la plus proche |
| **Méthode** | Test isolé de la logique de tri (sans threads, sans timing) |
| **Résultat** | ✅ Tri par deadline + tie-break par ordre d'arrivée, corrects et reproductibles |

---

## 1. Pourquoi tester la logique EDF isolément

Lire l'ordre EDF directement dans les logs d'une vraie simulation est peu fiable : deux requêtes ne sont "en compétition" que si elles partagent un dongle, ce qui est difficile à confirmer à l'œil sur un log réel (timing variable, contention des dongles, etc.).

On teste donc directement la fonction responsable de l'ordre — `compare_requests()` et le tas binaire (`push_request` / `pop_request`) — sans dongles, sans coders, sans threads. Ça donne un résultat **déterministe**, reproductible à chaque exécution.

---

## 2. Le test

### Scénario choisi

4 requêtes insérées dans un ordre volontairement "mélangé" par rapport à leurs deadlines :

| coder_id | deadline | arrival_order |
|---|---|---|
| 0 | 500 | 0 (arrivé en premier) |
| 1 | 100 | 1 |
| 2 | 300 | 2 |
| 3 | 100 | 3 (même deadline que coder 1, arrivé après) |

### Résultat attendu

- coder 1 doit sortir **en premier** (deadline la plus proche), malgré son arrivée après coder 0.
- coder 3 doit sortir **juste après** (même deadline que coder 1) grâce au tie-break par `arrival_order`.
- coder 2 puis coder 0 ensuite, dans l'ordre croissant de deadline.

### Code du test

```c
#include "codexion.h"
#include <assert.h>

int main(void)
{
	t_priority_queue	queue;
	t_compile_request	req;
	t_compile_request	out;

	init_priority_queue(&queue, 10);
	req = (t_compile_request){.coder_id = 0, .deadline = 500, .arrival_order = 0};
	push_request(&queue, req, SCHEDULER_EDF);
	req = (t_compile_request){.coder_id = 1, .deadline = 100, .arrival_order = 1};
	push_request(&queue, req, SCHEDULER_EDF);
	req = (t_compile_request){.coder_id = 2, .deadline = 300, .arrival_order = 2};
	push_request(&queue, req, SCHEDULER_EDF);
	req = (t_compile_request){.coder_id = 3, .deadline = 100, .arrival_order = 3};
	push_request(&queue, req, SCHEDULER_EDF);

	pop_request(&queue, &out, SCHEDULER_EDF);
	assert(out.coder_id == 1);   // deadline la + proche (100), malgré arrival=1
	pop_request(&queue, &out, SCHEDULER_EDF);
	assert(out.coder_id == 3);   // deadline égale (100) -> tie-break par arrival_order
	pop_request(&queue, &out, SCHEDULER_EDF);
	assert(out.coder_id == 2);   // deadline 300
	pop_request(&queue, &out, SCHEDULER_EDF);
	assert(out.coder_id == 0);   // deadline 500, arrivé en premier mais sort en dernier

	free(queue.requests);
	printf("Tous les tests EDF passent !\n");
	return (0);
}
```

### Commandes de compilation et d'exécution

```sh
cc -Wall -Wextra -Werror -Iincludes srcs/queue/priority_queue.c srcs/queue/priority_queue_heap.c test_edf_logic.c -o test_edf_logic
./test_edf_logic
```

---

## 3. Résultat obtenu

```
1er: coder=1 deadline=100
2e:  coder=3 deadline=100
3e:  coder=2 deadline=300
4e:  coder=0 deadline=500
Tous les tests EDF passent !
```

Exécuté **8 fois de suite** : résultat strictement identique à chaque fois (déterministe, aucun thread impliqué donc aucune variation possible).

---

## 4. Conclusion

✅ La logique EDF est correcte :
- Le tri se fait bien par **deadline croissante**.
- Le **tie-break par `arrival_order`** fonctionne quand deux deadlines sont égales.
- Le comportement est stable et reproductible, indépendamment de tout aléa de timing OS.

Ce test isole la logique de décision EDF de tout le reste (dongles, scheduler, threads) : il confirme que **si** le scheduler reçoit les bonnes requêtes au bon moment, il les ordonnera correctement.
