# <p style="text-align:center; color: Red">Rapport</p>
##### <p style="text-align:center;">Brahima Dibassi / Epron Quentin</p>
<hr>
#### <p style="color: #31c7c7;">Réalisation:</p>

 - Association de 2 languages différents
 - Implémentation complètes des tlv demandés
 - Librairies faites mains
 - Echange serveur / client simple d'utilisation
 - Puissance 4 fonctionnel
 - Un mode debug

#### <p style="color: #31c7c7;">Répartition du travail:</p>
- Client et tlv : Brahima
- Serveur et puissance 4 : Quentin

#### <p style="color: #31c7c7;">Structure du programme:</p>
- tcp : fonctions liées a l'initialisation du serveur et du client
- util_func : fonctions utiles non spécifique 
- server_func : fonctions appelées lorsque le serveur est en cours
- tlv : fonctions liées aux TLV
- p4 : fonctions liées au puissance 4
- client : fonctions du client, reception/transmission des tlv
- server : appel les fonctions liées au serveur

#### <p style="color: #31c7c7;">Remarque:</p>
> Quentin

J'ai eu du mal a implémenter la logique des tests pour determiner si le coup du joueur est gagnant ou non. Je pense que la solution que j'ai déduis être la meilleure n'est pas optimal et peux être revu mais c'est la solution qui ma parru le plus simple et la moins lourde en ligne de code

Grâce au tlv de Brahima implémenter la gestion des tlv pour le serveur à été particulièrement simple, cepenant il m'a fallu un petit peu de temps avant de bien comprendre la logique du serveur dans sa gestion des tlv.

Globalement concevoir un serveur qui utilise des tlv comme moyen de communication fut très enrichissant

> Brahima

Alors lors de ce projet j'ai trouvé globalement super fun et utile de crée un protocole réseau from scratch.

Grâce au cours très complet que l'on a eu, faire le projet final c'est déroulé sans difficulté.
