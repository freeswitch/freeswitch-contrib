<?php
    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                                easyroute

    Derniere mise a jour  :  22 aout  09
    Auteur                : InSiDe
    Dev 0.1
    ------------------------------------------------------------------------------*/


    require CHEMIN_LIB.    'fonction.paigner.php';
    include CHEMIN_MODELE. 'number.class.php';

    $head['titre'] = 'Liste des "Numbers".';

    // Instanciation de la classe Commentaires
    $number = new Number();

    // Récupération du nombre de commentaires
    $nombre_numbers = $number->countNumbers();

    // Préparation de la paignation
    $lien_paignation = ' <a href="./index.php?module=numbers&action=liste_numbers&page=%d"> %1$d </a> ';
    $paignation = paigner($nombre_numbers, NUMBERS_PAR_PAGE, $page, $lien_paignation);

    // Récupération des commentaires
    $premiereNumber = ($page -1) * NUMBERS_PAR_PAGE;
    $requete = $number->getAllNumbers($premiereNumber, NUMBERS_PAR_PAGE);

    // Affichage
    include CHEMIN_VUE. 'en-tete_liste_numbers.tpl.php';

    while($data = mysql_fetch_assoc($requete))
        include CHEMIN_VUE. 'colonne_number.tpl.php';

    include CHEMIN_VUE. 'bas_liste_numbers.tpl.php';

?>