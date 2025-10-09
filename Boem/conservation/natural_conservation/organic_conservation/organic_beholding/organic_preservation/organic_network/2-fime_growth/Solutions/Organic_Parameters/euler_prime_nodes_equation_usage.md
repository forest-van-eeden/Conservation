That expression —
nₑ = \lfloor k \times e \rfloor_{p^-}
— is a mathematical definition, so it’s compatible with any format that supports plain text, Markdown, or LaTeX-style math.

Here’s how to handle it depending on where you’re copying it:

⸻

🪶 1. Markdown (recommended for documentation or .md files)

You can paste it directly inside math delimiters:

nₑ = \lfloor k \times e \rfloor_{p^-}

or for inline:

$nₑ = \lfloor k \times e \rfloor_{p^-}$

This works perfectly in editors like GitHub, VS Code, or Obsidian.

⸻

📘 2. Code Files (e.g., .js, .py, .c)

Use it inside a comment:

// nₑ = floor(k * e) rounded down to nearest prime (⌊⋅⌋ₚ⁻)

This keeps it readable and preserves the idea.

⸻

🧮 3. LaTeX (.tex file)

Paste it as-is:

n_e = \lfloor k \times e \rfloor_{p^-}

and it will render properly in math mode.

⸻

🧱 4. Plain Text (.txt)

If you want a clean ASCII-safe version:

ne = floor(k * e) rounded down to nearest lower prime


⸻

If you tell me what environment you’re building the documentation or code in (e.g., Electron, VS Code, Xcode, etc.), I can give you the best exact syntax format for pasting this formula cleanly into your files.
Which one are you using right now?
