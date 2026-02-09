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
				
				if material.isEmpty { continue }
			
				//if material.hasPrefix	
		}
		
	}
	
}

provider()
