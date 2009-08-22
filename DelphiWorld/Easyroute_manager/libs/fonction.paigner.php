<?php


    function paigner($n_messages, $n_par_page, $page, $lien)
    {
        
  
        $adj = 3;
        $n_messages = (int) $n_messages;
        $n_par_page = (int) $n_par_page;
        $page       = (int) $page;
        
        $pagination = null;
        
        // S'il y a une ou aucune page
        if($n_messages == 0 OR $n_messages < $n_par_page)
            return null;
         
        
        $total = ceil($n_messages / $n_par_page);
        
        
        if ($total < 7 + ($adj * 2))
        {

           /* Pour les pages restantes on utilise une boucle for */
           for ($i = 1; $i <= $total; $i++)
            {
               if ($i == $page) // Le numéro de la page courante est mis en évidence (cf fichier CSS)
                    $pagination .= $i;
               else // Les autres sont affichés normalement
                    $pagination .= sprintf($lien, $i);
           }
          
        }

        /* CAS 2 : au moins 13 pages, troncature */
        else
        {
            /*
            Troncature 1 : on se situe dans la partie proche des premières pages, on tronque donc la fin de la pagination.
            l'affichage sera de neuf numéros de pages à gauche ... deux à droite (cf figure 1)
            */
            if ($page < 2 + ($adj * 2))
            {
                // Affichage du numéro de page 1 
                // puis des huit autres suivants */
                for ($i = 1; $i < (4 + ($adj * 2)); $i++)
                {
                    if ($i == $page)
                        $pagination .= $i;
                   else
                        $pagination .= sprintf($lien, $i);
                }
 
                /* ... pour marquer la troncature */
                $pagination .= ' ... ';

                /* et enfin les deux derniers numéros */
                $pagination .= sprintf($lien, ($total -1) );
                $pagination .= sprintf($lien, $total      );
            }
 
            /*
                Troncature 2 : on se situe dans la partie centrale de notre pagination, on tronque donc le début et la fin 
                de  la pagination.
                l'affichage sera deux numéros de pages à gauche ... sept au centre ... deux à droite (cf figure 2)
            */
            elseif ( (($adj * 2) + 1 < $page) && ($page < $total - ($adj * 2)) )
            {
                /* Affichage des numéros 1 et 2 */
                $pagination .= sprintf($lien, 1);
                $pagination .= sprintf($lien, 2);

                $pagination .= ' ... ';

                /* les septs du milieu : les trois précédents la page courante, la page courante, puis les trois 
                lui succédant */
                
                for ($i = $page - $adj; $i <= $page + $adj; $i++)
                {
                    if ($i == $page)
                        $pagination .= $i;
                    else
                        $pagination .= sprintf($lien, $i);
                }

                $pagination .= ' ... ';

                /* et enfin les deux derniers numéros */
                $pagination .= sprintf($lien, ($total -1) );
                $pagination .= sprintf($lien, $total      );
            }

            /*
            Troncature 3 : on se situe dans la partie de droite, on tronque donc le début de la pagination.
            l'affichage sera deux numéros de pages à gauche ... neuf à droite (cf figure 3)
            */
            else
            {
                /* Affichage des numéros 1 et 2 */
                $pagination .= sprintf($lien, 1);
                $pagination .= sprintf($lien, 2);

                $pagination .= ' ... ';
 
                /* puis des neufs dernières */
                for ($i = $total - (2 + ($adj * 2)); $i <= $total; $i++)
                {
                    if ($i == $page)
                        $pagination .= $i;
                    else
                    $pagination .= sprintf($lien, $i);
                }
            }
    }
    
    return '<br /> <div class="paignation"> Page : '.$pagination.' </div> <br />';
    }
?>