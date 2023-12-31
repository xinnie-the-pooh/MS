#include "include.fx"

Technique 
< 
	String	Name = "Inst: break, in rep, skip after break";
	String	Shape = "TinyQuad";
>
{
    Pass P0
    {        
 		
        VertexShaderConstant[0] = <mFinal>;
		       
        PixelShader =
        asm
        {
			ps_2_x
			def			c0,		1.0,	0.0,	0.0,	0.0
			defi		i0,		1,		0,		0,		0
			
			mov			r0.rgb, c0.y			// set the pixel color to black

			rep			i0
				mov			r0.r,	c0.x			// set the pixel color to red
				break
				mov			r0.r,	c0.y			// set the pixel color to black
			endrep
			
			mov			oC0,	r0
        };
         
		VertexShader = <StandardVS>;
   }
}

Technique 
< 
	String	Name = "Inst: break, in nested rep, skip after break";
	String	Shape = "TinyQuad";
>
{
    Pass P0
    {        
 		
        VertexShaderConstant[0] = <mFinal>;
		       
        PixelShader =
        asm
        {
			ps_2_x
			def 		c0,		.25,	0.0,	0.0,	0.0
			defi 		i0,		1,		0,		0,		0
			defi 		i1,		1,		0,		0,		0
			defi 		i2,		1,		0,		0,		0
			defi 		i3,		1,		0,		0,		0
			
			mov			r0.rgb, c0.y			// set the pixel color to black

			rep 		i0
				rep 		i1
					rep 		i2
						rep			i3
							mov			r0.r,	c0.x			// set the pixel color to red
							break
							mov 		r0.r,	c0.y			// set the pixel color to black
						endrep
						add 		r0.r,	r0.r, c0.x		// add the pixel color to red
						break
						mov 		r0.r,	c0.y			// set the pixel color to black
					endrep
					add 		r0.r,	r0.r, c0.x		// add the pixel color to red
					break
					mov 		r0.r,	c0.y			// set the pixel color to black
				endrep
				add 		r0.r,	r0.r, c0.x		// add the pixel color to red
				break
				mov 		r0.r,	c0.y			// set the pixel color to black
			endrep
			
			mov oC0, r0
        };
         
		VertexShader = <StandardVS>;
   }
}

Technique 
< 
	String	Name = "Inst: break, in rep, break to correct inst";
	String	Shape = "TinyQuad";
>
{
    Pass P0
    {        
 		
        VertexShaderConstant[0] = <mFinal>;
		       
        PixelShader =
        asm
        {
			ps_2_x
			def			c0,		1.0,	0.0,	0.0,	0.0
			defi		i0,		1,		0,		0,		0
			
			mov			r0.rgb, c0.y			// set the pixel color to black

			rep			i0
				break
			endrep
			mov			r0.r,	c0.x			// set the pixel color to red
			
			mov			oC0,	r0
        };
         
		VertexShader = <StandardVS>;
   }
}
