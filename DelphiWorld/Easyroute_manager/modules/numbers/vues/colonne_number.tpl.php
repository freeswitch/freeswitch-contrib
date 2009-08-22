    <tr>
        <td> <?php echo $data['number_id'] ?>  </td>
        <td> <?php echo $data['gateway_id'] ?>  </td>
        <td> <?php echo $data['number'] ?>       </td>
        <td> <?php echo $data['acctcode'] ?>       </td>
        <td> <?php echo $data['translated'] ?>  </td>
        <td>
            <a href="./index.php?module=numbers&action=modifier&id=<?php echo $data['number_id']?>">edit</a>
        </td>
        <td>
            <a href="./index.php?module=numbers&action=supprimer&id=<?php echo $data['number_id']?>">Delete</a>
        </td>
    </tr>