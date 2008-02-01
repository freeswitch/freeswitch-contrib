{section name=cnt loop=$conferences}
  {strip}
    <form method="post" action="{$smarty.server.PHP_SELF}">
    <input type="hidden" name="conf" value="{$conferences[cnt].name}"/>
    <input type="hidden" name="gateway" value="{$conferences[cnt].gateway}"/>
    <table id="fstable" cellspacing="0" summary="Conference list">
      <caption>{$conferences[cnt].title}</caption>
      <tr>
	<th scope="col" abbr=""></th>
        <th scope="col" abbr="ID">ID</th>
        <th scope="col" abbr="UUID">UUID</th>
        <th scope="col" abbr="CLID">CLID</th>
        <th scope="col" abbr="Flags">Flags</th>
        <th scope="col" abbr="in/out/energy">in/out/energy</th>
      </tr>
      {section name=cnt2 loop=$conferences[cnt].sessions}
        <tr class="{cycle values="normal,alt"}">
	  <td><input type="checkbox" name="memid[]" value="{$conferences[cnt].sessions[cnt2].member_id}"/></td>
          <td>{$conferences[cnt].sessions[cnt2].member_id}</td>
          <td>{$conferences[cnt].sessions[cnt2].uuid}</td>
          <td>{$conferences[cnt].sessions[cnt2].cid_name} &lt;{$conferences[cnt].sessions[cnt2].cid_num}&gt;</td>
          <td>{$conferences[cnt].sessions[cnt2].flags}</td>
          <td>{$conferences[cnt].sessions[cnt2].volume_in}/{$conferences[cnt].sessions[cnt2].volume_out}/{$conferences[cnt].sessions[cnt2].energy}</td>
        </tr>
      {/section}
    </table>
    <input class="submit" type="submit" name="action" value="kick">
    <input class="submit" type="submit" name="action" value="mute">
    <input class="submit" type="submit" name="action" value="unmute">
    <input class="submit" type="submit" name="action" value="deaf">
    <input class="submit" type="submit" name="action" value="undeaf">
    </form>
  {/strip}
{sectionelse}
There are currently no conferences active<br/>
{/section}
