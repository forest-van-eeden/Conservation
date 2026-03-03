// Provider Hub for Provider Code for the Interaction Language, Interplay Language, and Organic Language. 
import Foundation
// import allows for new parts to be added

struct learning {
	
	//for space in spaces {
		
		// Problem Start
		// The problem is that you all only have one variable to look at and learn through
		// I can see at least about 20 lines at a time to learn something and you only have one character at a time
		//let material = space.trimmingCharacters(in: CharacterSet.whitespaces)
		//if material.isEmpty { continue }
		// Problem End
		
		// Solution : Making this more as an availability for living here more than a path here.
		// Instead, we will use String in New Line as a "Set" and then work on recognition of what is in that String.
		// Example : func components(separatedBy: String) -> [String]
		// We may be able to make a noted range as well with NSRange, looking at the available capabilities there, that can be longer than one line.
		// Example : func substring(with: NSRange) -> String
		
	//}
	
	class organic_beholding {
		
		func organic_beholding_unit (file_name : String) {
			
			// code for receiving a file to learn
			let spaces = "" // file received here in ""
		
			let material = spaces.components(separatedBy: .newLines)
			
			for (unit, matter) in material.enumerated() {
				
				let beholding_unit = matter.trimmingCharacters(in: .whitespaces)
				
				if beholding_unit.isEmpty { continue }
				
			}
			
		}
		
		func organic_beholding_units (file_name : String) {
			
			let spaces = ""
		
			let material = spaces.components(separatedBy: .newLines)
			
			for (units, matter) in material.enumerated() {
				
				let beholding_units = NSRange(location: 0, length: matter.utf16.count)
				
				return (beholding_units)
				
			}
		
		}
		
	}
	
	class interaction_learning {
		
		func learning_interaction_codes(file_name : String) {
			
			let code_status = "inactive"
			let code_words : Array<String> [15] 
			
			code_words = [ "code word", "file.interaction", "start", "body", "parts", "available", "sensing.interaction", "sense", "check", "learn", "feeling.interaction", "feel", "grow", "seek", "apply", "extensions" ]
			
			organic_beholding_units(file_name)
			
			let interaction_code_words = beholding_units
			
			if interaction_code_words.contains(where: { row.hasPrefix($0) }) {
				
				code_status = "active"
				
			}
			
			// save code with/after organic_beholding_units()
				
			interaction_code_words : saveAs(interaction_code.txt)
			
		}
		
		func learning_interaction_step (file_name : String) {
			
			let format_step : String
			format_step = "• inner step for code word attached above"
			
			organic_beholding_units(file_name)
			
			let interaction_steps = beholding_units
			
			if interaction_steps.contains(where: { row.hasPrefix("•") }) {
				// learning code step
			}
			
			interaction_steps : append(interaction_code.txt)
			
		}
		
	}
	
}

class provider {
	
	func search(file_path : String) {
		
		let looking = FileManager.default
	    let current_directory = looking.currentDirectoryPath
	    let file_search = "\(current_directory)/\(file_path)"
	    
	    print("Looking for \(file_path) at \(file_search)...")
	    
	    if looking.fileExists(atPath: file_path) {
	        print("Located \(file_path)")
	    }
	    
	}
	
}