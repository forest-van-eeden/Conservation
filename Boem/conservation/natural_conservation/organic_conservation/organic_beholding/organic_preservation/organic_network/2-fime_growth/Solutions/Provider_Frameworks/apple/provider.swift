import Foundation

func provider() {
	
	// Call File Manager
	let looking = FileManager.default
	
	// Looking Path
	let current_directory = looking.currentDirectoryPath
	
	// Name Format File
	let interaction_format = "format.interaction"
	
	// Format Path
	let format_path = "\(current_directory)/Interaction/\(interaction_format)"
	
	print ("Looking for format.interaction at \(format_path)...")
	
	if looking.fileExists(atPath: format_path) {
		
		print ("Located interaction.format")

		// Accessing the Format
		let format = try String(contentsOfFile: format_path, encoding: .utf8)
		
		// Learning the Format
		let spaces = format.components(separatedBy: .newlines)
		
		for space in spaces {

				// Defining Material Space
				let material = space.trimmingCharacters(in: .whitespaces)			
				if material.isEmpty { continue }

				// Learning Code Words Space
				var code_word: String = ""
				// if material.hasNoPrefix { make into code_word }
				// code_word is like a function notation
				// if code_word { a step can be underneath }

				// Learning Steps Space
				if material.hasPrefix("•") { 
					let step = material.replacingOccurrences(of: "•", with: "").trimmingCharacters(in: .whitespaces)
				}
				// if step { this is a task to do as if inside of the code_word like it was a function }
				// steps are to be done in order of appearance
			
				// Learning File Extensions Space
				// file.interation is a code_word
				if code_word.contains(".interaction") {
					print("Learning \(code_word)")
				} else {
					code_word = material
				}
				// the step is for the provider to then go to a file that is input that way in an .interaction document
		}
		
	}
	
}

provider()
