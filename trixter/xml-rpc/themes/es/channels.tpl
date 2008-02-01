{if is_array($channels)}
<table id="fstable" cellspacing="0" summary="Channel list">
<caption>Channel List</caption>
  <tr>
    <th scope="col" abbr="UUID">UUID</th>
    <th scope="col" abbr="Start">Comienzo</th>
    <th scope="col" abbr="From">De</th>
    <th scope="col" abbr="To">A</th>
    <th scope="col" abbr="Route">Ruta</th>
  </tr>
{section name=cnt loop=$channels}
{strip}
   <tr class="{cycle values="normal,alt"}">
      <td>{$channels[cnt].uuid}</td>
      <td>{$channels[cnt].created}</td>
      <td>{$channels[cnt].cid_name} &lt;{$channels[cnt].cid_num}&gt;</td>
      <td>{$channels[cnt].dest}</td>
      <td>{$channels[cnt].name}</td>
   </tr>
{/strip}
{/section}
</table>
<br/>
{else}
There are currently no channels active<br/>
{/if}
