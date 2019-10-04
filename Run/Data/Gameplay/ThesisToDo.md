##THESIS TO DO##

	-	Finalize sub section WFC Test
	-	Implement a modified WFC Architecture
		- Implement a RGBA8 class
		- Use the RGBA8 class to work with the WFC model
		- Use Alpha component to be a probabilistic weight for different possible outcomes
	-	Test object/obstacle WFC with the overlapping model
	- 	Test object/obstacle WFC model with the modified WFC model

	-	Create sub room WFC for following room layouts
		-	Office Cabin
		-	Conference Room
		-	Pantry
		-	Bathrooms
		-	Lounge Room
		-	Storage Room

##NOTES##

	-	Metrics need to be developed once generation has been finalized
	-	Create a modified WFC to contrast between classic/vanilla WFC
	-	Scenario of office area (For the scope of this project)
	-	Wet wall maintenance and structural rigidity
	-	Can we support multiple different floors?
	-	Can we support non rectangular layouts? If yes what are we doing to split the map into sections

##From Conversation on 02-10-2019##

	-	Think about using an RNG that relies on memory (look at Sobol RNG)
	- 	If you're making a race track (Ludem Dare) think about how to influence the map using difficulty
	-	The difficulty should account for the pattern and use some form of weighting 
	-	You don't want an already difficult road like a curve to have obstacles on it (like tar, oil) you want those
		on easier roads and paths instead
	-	You could try using the tiled WFC model with multiple tiled outputs to create different paths and see what that gives you
	- 	Road generation is a good use case of simple tiled model if you define the tile to exist in the shape that it is
	-	You need to implement the Modified WFC algorithm that takes the 4 components and uses Alpha for weighting