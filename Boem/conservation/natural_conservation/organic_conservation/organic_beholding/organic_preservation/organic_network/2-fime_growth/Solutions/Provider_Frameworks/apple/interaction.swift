// Interaction Language Provider Code
import Foundation

let provider = provider()
let format = provider.search(file_path: "Interaction/format.interaction")

struct learning {

	class format {
		
		func learn_format() {
		    
		        // 1. OPENING THE BOX (Unwrapping)
		        // We use 'try?' to attempt reading, and 'if let' to ensure it worked.
		        let format_path = "Interaction/format.interaction"
		        if let learning = try? String(contentsOfFile: format_path, encoding: .utf8) {
		            
		            // Now 'format' is a real String, so we can use .components
		            let spaces = learning.components(separatedBy: .newlines)
		            
		            // Define the context memory OUTSIDE the loop
		            var current_code_word: String = "None"
		            
		            for space in spaces {
		                // 2. CLARIFYING THE CATEGORY (CharacterSet)
		                let material = space.trimmingCharacters(in: CharacterSet.whitespaces)
		                
		                if material.isEmpty { continue }
		
		                if material.hasPrefix("•") { 
		                    let step = material.replacingOccurrences(of: "•", with: "").trimmingCharacters(in: CharacterSet.whitespaces)
		                    print("ACTION: Under [\(current_code_word)], performing: \(step)")
		                } 
		                else {
		                    // This identifies your standalone Code Word
		                    current_code_word = material
		                    print("\nCONTEXT: \(current_code_word)")
		                }
		            }
		        } else {
		            print("ERROR: File exists but the computer could not read the text.")
		        }
		    } else {
		        print("ERROR: File not found at \(format_path)")
		    }
		}
		
	}
	
	func learn_parts() {
		
	}
	
	func interaction() {
		
		search()
		learn_format()
		
	}

}