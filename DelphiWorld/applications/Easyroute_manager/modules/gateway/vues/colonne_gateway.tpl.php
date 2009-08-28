

    <tr>
        <td> <?php echo $data['gateway_id'] ?>  </td>
        <td> <?php echo $data['gateway_ip'] ?>  </td>
        <td> <?php echo $data['group'] ?>       </td>
        <td> <?php echo $data['limit'] ?>       </td>
        <td> <?php echo $data['techprofile'] ?>  </td>
        <td>
            <a href="./index.php?module=gateway&action=modifier&id=<?php echo $data['gateway_id']?>">Modifier</a>
        </td>
        <td>
            <a href="./index.php?module=gateway&action=supprimer&id=<?php echo $data['gateway_id']?>">Supprimer</a>
        </td>

        <td>
            <a href="./index.php?module=numbers&action=ajouter&id=<?php echo $data['gateway_id']?>">Number</a>
        </td>
    </tr>