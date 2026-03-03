import SwiftUI
import Combine
import CoreMotion

//maybe Firebase next time, I tried my best this life
//compiling in terminal: swiftc -parse-as-library interpanel.swift -o interpanel.app/Contents/MacOS/interpanel

struct ContentView: View {
	
	var body: some View {
		
		ZStack {
		
			interpanel()
			
			interspace()
			
			
		}
		.frame(minWidth: 907, minHeight: 739)
		.background(Color.black)
		
	}
}

@main

struct window: App {
	
	var body: some Scene {
		
		WindowGroup {
			
			ContentView()
			
		}
		
	}
	
}

struct interpanel: View {
	
	var body: some View {
		Path { border in
					
			border.move(to: CGPoint(x: 173, y: 0))
			border.addLine(to: CGPoint(x: 853, y: 0))
			border.addLine(to: CGPoint(x: 853, y: 179))
			border.addLine(to: CGPoint(x: 0, y: 179))
			border.addLine(to: CGPoint(x: 0, y: 733))
			border.addLine(to: CGPoint(x: 173, y: 733))
			border.addLine(to: CGPoint(x: 173, y: 0))
			border.closeSubpath()
					
		}
			.stroke(Color(hex: "#E66C02"), lineWidth: 5)
				
		Button(action: {
			//Hold for page opening actions
			print("Boem App Accessed")
		}){
					
			Text("BOEM")
				.font(.system(size: 19, weight: .bold, design: .monospaced))
				.foregroundColor(Color(hex: "#00EE82"))
				.padding(.vertical, 19)
				.padding(.horizontal, 59)
				.background(Color.black)
				.overlay(
					Rectangle()
						.stroke(Color(hex: "#00EE82"), lineWidth: 2)
				)
					
		}
			.buttonStyle(PlainButtonStyle())
			.rotationEffect(.degrees(-90))
			.offset(x: -241, y: -277)
			
	}
			
}

struct interspace: View {
	
	var body: some View {
		
		ZStack {
			
			person()
			
		}
		
	}
	
}

struct person: View {
	
	@State private var pulse: CGFloat = 1.0
	
	var body: some View {
		
		TimelineView(.periodic(from: .now, by: 1/60)) { context in
				
			let time = context.date.timeIntervalSinceReferenceDate
				
			let breathScale = 1.0 + (0.1 * sin(time * 1.5))
		
			ZStack {
				
				Circle()
					.fill(RadialGradient(
						gradient: Gradient(colors: [Color(hex: "#00EE82").opacity(0.7), .clear]),
						center: .center,
						startRadius: 5,
						endRadius: 116.5 * breathScale
					))
					.frame(width: 233 * breathScale, height: 233 * breathScale)
					.position(x: 521, y: 443)
					
			}
			
		}
		
	}
	
}

extension Color {
	
	init(hex: String) {
		
		let hex = hex.trimmingCharacters(in: CharacterSet.alphanumerics.inverted)
		var int: UInt64 = 0
		Scanner(string: hex).scanHexInt64(&int)
		let a, r, g, b: UInt64
		switch hex.count {
			case 3:
				(a, r, g, b) = (255, (int >> 8) * 17, (int >> 4 & 0xF) * 17, (int & 0xF) * 17)
			case 6:
				(a, r, g, b) = (255, int >> 16, int >> 8 & 0xFF, int & 0xFF)
			default:
				(a, r, g, b) = (1, 1, 1, 0)
		}
		self.init(.sRGB, red: Double(r) / 255, green: Double(g) / 255, blue: Double(b) / 255, opacity: Double(a) / 255)
		
	}
	
}
