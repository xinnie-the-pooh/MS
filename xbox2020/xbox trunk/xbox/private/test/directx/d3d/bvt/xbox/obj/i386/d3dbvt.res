L /��<�  
      .debug$S        E   �               @ B.rsrc$01        �   �   �         @  @.rsrc$02          �              @  @    	     obj\i386\d3dbvt.res#    Microsoft CVTRES 5.00.2134.1                
      �   @  �               �  �X  ��  �p  �                  �  �               	  �                  	  �                  	  �       k              [              <           V S H A D E R  P S H A D E R �       �       �   	    ;
; Vertex input registers:
;
; v0            Vertex position
; v3            Vertex normal
; v7            Vertex texture coordinates
;
; Constant registers:
;
; c0 - c3       Transposed concatenation of world, view, and projection matrices
;
; c4            Normalized light direction
; c5            View position
; c6.x          Material power
; c6.y          0.0f
; c6.z          1.0f
; c7            Diffuse base color (diffuse light color modulated with diffuse material color)
; c8            Diffuse offset color (ambient room color modulated with ambient material color +
;                                     ambient light color modulated with ambient material color +
;                                     emissive material color)
; c9            Specular base color (specular light color modulated with specular material color)
;

; Transform the vertex

dp4 oPos.x, v0, c0
dp4 oPos.y, v0, c1
dp4 oPos.z, v0, c2
dp4 oPos.w, v0, c3

; Light the vertex with a directional light

dp3 r2.x, v3, -c4                   ; Diffuse intensity = dotproduct(vertex normal, light direction)

add r0, v0, -c5                     ; Calculate a vector from the eye to the vertex
nm3 r0, r0
add r1, r0, c4                      ; Calculate the half-vector
nm3 r1, r1
dp3 r2.y, v3, -r1                   ; Specular intensity = dotproduct(vertex normal, light/eye half-vector)

mov r2.w, c6.x                      ; Material power

lit r3, r2                          ; Light coefficients

mad r0.xyz, r3.y, c7.xyz, c8.xyz    ; Diffuse color = diffuse intensity * diffuse base color + diffuse offset color
mov r0.w, c7.w                      ; Diffuse alpha = diffuse base alpha
min oD0, r0, r3.w                   ; Clamp diffuse colors to 1.0f

mul r0.xyz, r3.z, c9.xyz            ; Specular color = specular intensity * specular base color
mov r0.w, c9.w                      ; Specular alpha = specular base alpha
min oD1, r0, r3.w                   ; Clamp specular colors to 1.0f

mov oT0, v7                         ; Propagate texture coordinates

; Light intensity factored into alpha calculations

;dp3 r2.x, v3, -c4       ; Diffuse intensity = dotproduct(vertex normal, light direction)

;add r0, v0, -c5         ; Calculate a vector from the eye to the vertex
;nm3 r0, r0
;add r1, r0, c4          ; Calculate the half-vector
;nm3 r1, r1
;dp3 r2.y, v3, -r1       ; Specular intensity = dotproduct(vertex normal, light/eye half-vector)

;mov r2.w, c6.x          ; Material power

;lit r3, r2              ; Light coefficients

;mad r0, r3.y, c7, c8    ; Diffuse color = diffuse intensity * diffuse base color + diffuse offset color
;min oD0, r0, r3.w       ; Clamp diffuse colors to 1.0f

;mul r0, r3.z, c9        ; Specular color = specular intensity * specular base color
;min oD1, r0, r3.w       ; Clamp specular colors to 1.0f
     ;
; Color input registers:
;
; v0            Diffuse color
; v1            Specular color
;

tex t0              ; Sample the texture set in stage 0
mul r1, t0, v0      ; Modulate the pixel's diffuse color with the texel color
add r0.xyz, r1, v1  ; Add the pixel's specular color
mov r0.a, r1.a      ; Use the modulated diffuse/texel alpha for the pixel     <4   V S _ V E R S I O N _ I N F O     ���       0    0?   	                    �   S t r i n g F i l e I n f o   x   0 4 0 9 0 4 B 0   L   C o m p a n y N a m e     M i c r o s o f t   C o r p o r a t i o n   v '  F i l e D e s c r i p t i o n     X B o x   D i r e c t 3 D   B u i l d   V e r i f i c a t i o n   T e s t s     8   F i l e V e r s i o n     1 . 0 0 . 4 4 0 0 . 1   .   I n t e r n a l N a m e   d 3 d b v t     t (  L e g a l C o p y r i g h t   C o p y r i g h t   ( C )   M i c r o s o f t   C o r p .   1 9 8 1 - 2 0 0 1   6   O r i g i n a l F i l e n a m e   d 3 d b v t     L   P r o d u c t N a m e     M i c r o s o f t ( R )   X b o x ( T M )   <   P r o d u c t V e r s i o n   1 . 0 0 . 4 4 0 0 . 1   D    V a r F i l e I n f o     $    T r a n s l a t i o n     	�    @comp.idV ��   .debug$S       E                 .rsrc$01       �                .rsrc$02                       $R000000        $R000B60`      $R000CD0�          