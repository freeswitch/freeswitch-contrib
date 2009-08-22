<?php
    $temps_debut = microtime(true);

    // Initialisation
    include 'global/init.php';

    // Début de la tamporisation de sortie
    ob_start();

    $page    = !empty($_GET['page'])   ? (int) $_GET['page']    : 1 ;

    // Si un module est specifié, on regarde s'il existe
    if (!empty($_GET['module']))
    {
        $module = dirname(__FILE__).'/modules/'.$_GET['module'].'/';

        // Si l'action est specifiée, on l'utilise, sinon, on tente une action par défaut
        $action = (!empty($_GET['action'])) ? $_GET['action'].'.php' : 'index.php';

        // Si l'action existe, on l'exécute
        if (is_file($module.$action))
                include $module.$action;

        // Sinon, on affiche la page d'accueil !
        else
                include 'global/module_introuvable.php';
    }

    // Module non specifié ou invalide ? On affiche la page d'accueil !
    else
        include 'modules/accueil/accueil.php';

    // Fin de la tamporisation de sortie
    $contenu = ob_get_clean();


    // Début du code HTML
    include 'global/haut.php';

    echo $contenu;

    // Fin du code HTML
    include 'global/bas.php';
?>