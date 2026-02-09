import Foundation

func provider() {
	
	// Call File Manager
	let file_manager = FileManager.default
	
	// Looking Path
	let current_path = file_manager.currentDirectoryPath
	
	// Name Format File
	let interaction_format = "format.interaction"
	
	// Format Path
	let format_path = "\(current_path)/Interaction/\(interaction_format)"
	
	print ("Looking for format.interaction at \(format_path)...")
	
	if file_manager.fileExists(atPath: format_path) {
		
		print ("Located interaction.format")

		// Accessing the Format
		let format = try String(contentsOfFile: format_path, encoding: .utf8)
		
		// Learning the Format
		let spaces = format.components(separatedBy: .newlines)
		
		for space in spaces {
			
				let material = space.trimmingCharacters(in: .whitespaces)

				// Need : what are .trimmingCharacter and .whitespaces
			
				if material.isEmpty { continue }

				// Learning Code Words
				// if material.hasNoPrefix { make into code_word }
				// code_word is like a function notation
				// if code_word { a step can be underneath }
				
				// if material.hasPrefix("•") { make into step }
				// if step { this is a task to do as if inside of the code_word like it was a function }
				// steps are to be done in order of appearance

				// file.interation is a code_word
				// the step is for the provider to then go to a file that is input that way in an .interaction document
		}
		
	}
	
}

provider()
