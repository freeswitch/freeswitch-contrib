{if is_array($errors)}
<fieldset class="boxit"><legend>Errors</legend>
{section name=cnt loop=$errors}
{$errors[cnt]}<br/>
{/section}
</fieldset>
<br/>
{/if}
