package assets.skins
{
	import flash.display.DisplayObject;
	
	import mx.containers.Panel;
	import mx.core.EdgeMetrics;
	import mx.core.mx_internal;
	import mx.skins.halo.PanelSkin;
	import mx.styles.CSSStyleDeclaration;
	import mx.styles.IStyleClient;
	import mx.styles.StyleManager;
	use namespace mx_internal;
	
	public class SmartPanelSkin extends PanelSkin
	{
		private var backgroundComplete:Boolean;
		private var contentComplete:Boolean;
		
		public function SmartPanelSkin()
		{
			super();
		}
		override mx_internal function drawBorder(w:Number, h:Number):void
		{
			if(!parent || contentComplete) return;
			
			contentComplete = true;
			var contentStyleName:* = getStyle("contentStyleName");
			if(contentStyleName)
			{
				var contentCSS:CSSStyleDeclaration = StyleManager.getStyleDeclaration("." + contentStyleName);
				var contentSkin:Class = contentCSS.getStyle("skin");
				if(contentSkin && parent is Panel)
				{
					var contentInstance:DisplayObject = new contentSkin();
	    			if(contentInstance is IStyleClient) IStyleClient(contentInstance).styleName = contentCSS;
	    			var metrics:EdgeMetrics = borderMetrics;
	    			contentInstance.width = w - metrics.left - metrics.right;
	    			contentInstance.height = h - metrics.bottom - metrics.top;
	    			contentInstance.x = metrics.left;
	    			contentInstance.y = metrics.top;
	    			var panel:Panel = Panel(parent);
	    			panel.rawChildren.addChildAt(contentInstance,1);
				}
			}
			else
			{
				super.drawBorder(w,h);
			}
		}
		override mx_internal function drawBackground(w:Number, h:Number):void
    	{
    		if(!parent || backgroundComplete) return;
    		
    		backgroundComplete = true; 
    		var backgroundSkin:Class = getStyle("backgroundSkin");
    		if(backgroundSkin && parent is Panel)
    		{
    			var backgroundInstance:DisplayObject = new backgroundSkin();
    			if(backgroundInstance is IStyleClient) IStyleClient(backgroundInstance).styleName = parent;
    			backgroundInstance.width = w;
    			backgroundInstance.height = h;
    			var panel:Panel = Panel(parent);
    			panel.rawChildren.addChildAt(backgroundInstance,0);
    		}
    		else
    		{
    			super.drawBackground(w,h);
    		}
    	}
	}
}