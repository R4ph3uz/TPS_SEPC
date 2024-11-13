# Sujets de SEPC
## Malloc 


Nous avons implémenté les petits, moyens et gros. Attention à la fonction mark_memarea_and_get_user_ptr qui peut etre tricky avec la logique de pointeurs. Vous pouvez décommenter pour voir comment deviennent les nombres (magique ou kind) en fonction des autres malloc

Cependant nous n'avons pas réussi à effectuer le test sur 32 bit.

D'après ce que nous avons compris, il faut ajouter au Cflag du CmakeList -m32 pour compiler en 32 bit et si c'est possible d'executer sur une machine 64 bit c'est bon. Sinon on peut passer par qemu...

Et on s'est pas chauffé pour le multi-thread. (deja pas mal de retard).

## Shell

Pour rattraper on est allé jusqu'à Excellent sur Shell (parce qu'on s'était séparer pour que je ne fasse pas l'AOD Voir autre repo).

Le shell passe tous les tests et toutes les variantes sont implémantées.

A noter:

Affiche Jobs n'affiche que ceux qui sont en attente encore et ne les attend pas grace à WNOHANG

les sigaction permettent de définir le handler (la fonction qui gère) lorsque le signal est émis (SIGCHLD par exemple)

les jokers sont ajoutés que si on le trouve dans la commande en cours (bien après le parsing de la commande donc)

La commande Ulimit est defaillante mais kill quand meme au bout du temps donné.

J'ai laissé les sources qui m'ont aidées pour implémenter mon shell & pipes & fichiers.

Puis globalement tout est commenté.




Envoyez moi un petit mail à r4ph3uz-github@yahoo.com si cela vous a aidé ;)
