package assets.skins
{
	import com.asfusion.skins.filters.ICSSFilter;
	import com.degrafa.Surface;
	
	import flash.events.Event;
	import flash.filters.BitmapFilter;
	
	import mx.styles.CSSStyleDeclaration;
	import mx.styles.IStyleClient;
	import mx.styles.StyleManager;
	
	public class BaseSkin extends Surface
	{
		[Bindable (event="skinSizeChange")]
		protected var skinWidth:int;
		
    	[Bindable (event="skinSizeChange")]
    	protected var skinHeight:int;
    	
    	//.........................................BaseSkin Contructor........................................
		public function BaseSkin()
		{
			super();
		}
    	
		//.........................................updateDisplayList..........................................
		override protected function updateDisplayList(unscaledWidth:Number, unscaledHeight:Number):void
		{
			super.updateDisplayList(unscaledWidth, unscaledHeight);
			if(unscaledWidth || unscaledHeight)
			{
				skinWidth = unscaledWidth;
				skinHeight = unscaledHeight;
				dispatchEvent( new Event("skinSizeChange"));
			}
		}
		
		//.........................................styleName..........................................
		override public function set styleName(value:Object):void
		{
			super.styleName = value;
			if(!value is IStyleClient) return;
			
			var bitmapFilters:Array = new Array();
			var CSSfilters:Object;
			if(value is IStyleClient)
			{
				CSSfilters = IStyleClient(value).getStyle('filters');
			}
			else if(value is CSSStyleDeclaration)
			{
				CSSfilters = CSSStyleDeclaration(value).getStyle('filters');
			}
			if(CSSfilters is String)
			{
				bitmapFilters.push(createFilter(CSSfilters as String));
			}
			else
			{
				for each( var filterStyleName:String in bitmapFilters)
				{
					bitmapFilters.push(createFilter(filterStyleName));
				}
			}
			filters = bitmapFilters;
		}
		
		//.........................................createFilter..........................................
		protected function createFilter(styleName:String):BitmapFilter
		{
			var CSSfilter:CSSStyleDeclaration = StyleManager.getStyleDeclaration("."+styleName);
			var filterClass:Class = CSSfilter.getStyle('filterClass');
			var filter:ICSSFilter = new filterClass();
			filter.styleName = CSSfilter;
			return filter.filter;
		}
		
		//.........................................getBottomCornerRadius..........................................
		protected function getBottomCornerRadius():int
    	{
    		var bottomCornerRadius:* = getStyle('bottomCornerRadius');
    		var bottomCorners:int = 0;
    		if(bottomCornerRadius !== 0)
    		{
    			bottomCorners = bottomCornerRadius;
    		}
    		return bottomCorners;
    	}
	}
}