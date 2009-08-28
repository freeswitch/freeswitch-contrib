<?php
    define('MODE_DEBUG',      true);
  define('SQL_DSN',      'localhost');
    define('SQL_USERNAME', 'root'     );
    define('SQL_PASSWORD', 'root'  );
    define('SQL_BDD',      'fs2bill'  );

    // Chemins à utiliser pour accéder aux vues/modeles/librairies
    $module = empty($module) ? !empty($_GET['module']) ? $_GET['module'] : 'accueil' : $module;


    // -- Chemins
    define('CHEMIN_VUE'         , 'modules/'.$module.'/vues/');
    define('CHEMIN_VUE_GLOBALE' , 'vues_globales/'           );
    define('CHEMIN_MODELE'      , 'modeles/'                 );
    define('CHEMIN_LIB'         , 'libs/'                    );

    define('GATEWAYS_PAR_PAGE'  , 15);
    define('NUMBERS_PAR_PAGE'  , 15);

?>