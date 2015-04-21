; Function declarations
(define (Appropo x) 
	(set! x (* x 17))
)
   
; Variable declarations
(let* 
	( 
		(a 6)  
		(b 3) 
		(c 7)
		(x '(1 3 6 "GIMP"))
	)   
	(set! a (+ a b))  
	(Appropo a)
	x
	(set! x (cons 1 x))
	(set! x (cons c x))
	(set! x (cons c x))
	(set! x (cons 1 x))
	(set! x (cons 1 x))
	x
)

 (let* 
	(
		(x  '( 
				(1 2(3 4 5)6)  
				7  
				8  
				(9 10) 
			)
		)
	)
	(car (car (cdr (cdr (car x)))))
	(caddar x)
	; place your car/cdr code here
)

; Creates a layer with set name in target image. Returns target layer
(define 
	(createNewLayer
		image
		name
	)
	(let* 
		; Variable declarations
		(
			(newLayer 1)
		)
		; Do stuff
		(set! newLayer (car (gimp-layer-new image 100 100 1 name 100 0)))
		(gimp-image-insert-layer image newLayer 0 -1)
		newLayer
	)
)

; (define (createNewLayer image name )	(let* ((newLayer 1)) (set! newLayer (car (gimp-layer-new image 100 100 1 name 100 0)))(gimp-image-insert-layer image newLayer 0 -1) newLayer )) (let* ( (image 1) (layer 1)  (layerName "") )   (set! image (car (gimp-image-new 50 50 RGB)))  (set! layer (car (gimp-layer-new image 50 50 1 "3" 100 0)))  (gimp-image-insert-layer image layer 0 -1)    (set! layerName (number->string 1))         (set! layer (gimp-image-get-layer-by-name image layerName))  (gimp-display-new image)   (if (negative? (car layer))  (createNewLayer image layerName)  3)       )

; GIMP main function declaration
(define 
	(script-fu-Layer-Merger 
		; Own image and layer args
		image
		drawable
		; Own args
		createNewImage
		saveToFile		
		flatten
	)
	
	; Start undo batch in case things fuck up
	(gimp-image-undo-group-start image)
	; Actual function process, yo.
	(let*
		; Variable declarations
		(
			; Get image size
			(width (car (gimp-image-width image))) 
			(height (car (gimp-image-height image)))
			(newImage 0)
			(newLayer 0)
			(mergeLayer 0)
			(newImageWidth 0)
			(newImageHeight 0)
			(layers )
			(numLayers)
			(layerIDs)
			; More or less temporary variables below
			(i)
			(layerName)
			(layerID 0)
			(restLayerList)
			(item)
			(layer)
			(row 0)
			(column 0)
		)
		(set! newImageWidth (* width 16))
		(set! newImageHeight (* height 16))
		(set! newImage (car (gimp-image-new newImageWidth newImageHeight RGB)))
		(set! mergeLayer (car (gimp-layer-new newImage newImageWidth newImageHeight 1 "MergedLayer" 100 0)))
		(gimp-image-insert-layer newImage mergeLayer 0 -1)
		(gimp-display-new newImage)
		
		; test
		; (let* ( (image 1) (layer 1)  )   (set! image (car (gimp-image-new 50 50 RGB)))  (set! layer (car (gimp-layer-new image 50 50 1 "3" 100 0)))  (gimp-image-insert-layer image layer 0 -1)   (gimp-display-new image)  ) 
		
		; Fetch layer-data
		(set! layers (gimp-image-get-layers image))
		(set! numLayers (car layers))
		(set! layerIDs (cdr layers))
		(set! restLayerList layerIDs)
		; Set i to 0 and go up to numLayers
		(set! i 0)
		; Loop over valid indices
		(while (< i 255)
			(set! i (+ i 1))
			; Look for layer with this ID.
			(set! layerName (number->string i))
			(set! layer (car (gimp-image-get-layer-by-name image layerName)))
			
			(if (positive? layer)
				; If an OK layer
				(begin 
					(set! newLayer (car (gimp-layer-new-from-drawable layer newImage)))
					(gimp-image-insert-layer newImage newLayer 0 -1)
					; Move it to its correct position
					(set! row (floor (/ i 16)))
					(set! column (modulo i 16))
					(gimp-layer-set-offsets newLayer  (* column width) (* row height))
					; Make it visible if it wasn't
					(gimp-item-set-visible newLayer TRUE)
				)
				; Else here
				(begin 
					-1;
				;	(createNewLayer newImage layerName)
				)
			)
		)	
		; Flatten image
		(if (= flatten TRUE)
			(begin 
				(gimp-image-merge-visible-layers newImage 1)
			)
		)
	)
	; Undo batch completion
	(gimp-image-undo-group-end image)
)

; Register it with the gIMP
(script-fu-register
	; Function name
	"script-fu-Layer-Merger"
	; Menu label
	"Layer Merger"
	; Description
	"Merges all layers in a tile-based fashion into a new image. By default using 16x16 tiles. The layers names will be used to index them correctly into the final image."
	; Author
	"Emil Hedemalm"
	; Copyright notice
	"Lall Copy right mee"
	; Date created
	"2014-02-06"
	; Image type that the script works on
	""
	; Uh... optional variables?
	SF-IMAGE	"Image"  	3		; Active image
	SF-DRAWABLE "Layer" 	17
	SF-TOGGLE	"Create new image" 	TRUE
	SF-TOGGLE	"Save to file" 		FALSE
	SF-TOGGLE 	"Flatten layers"	TRUE
;	SF-STRING "Text" "Text Box"
  ;    	     SF-STRING      "Text"          "Text Box"   ;a string variable
;    SF-FONT        "Font"          "Charter"    ;a font variable
 ;   SF-ADJUSTMENT  "Font size"     '(50 1 1000 1 10 0 1)
                                                ;a spin-button
 ;   SF-COLOR       "Color"         '(0 0 0)     ;color variable
)
; Register menu stuffs
;(script-fu-menu-register "script-fu-Layer-Merger" "<Image>/File/Create/Image")
(script-fu-menu-register "script-fu-Layer-Merger" "<Image>/Script-Fu/Layer Magic")
	  
	  
	  
	  
	  
	  
	  
	  