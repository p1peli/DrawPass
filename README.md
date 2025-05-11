# DrawPass
Generating entropy based on user input to create secure, unpredictable passwords. <br>
[🔗 Try it here](https://p1peli.github.io/DrawingPasswordGenerator/src/index.html) 

**DrawPass** is a browser-based password generator that allows you to generate a high-entropy password, by drawing on a canvas. 
Most traditional generators rely on pseudo-random number generators, which, while useful, are ultimately deterministic, and have already been exploited in the past [(e.g., Joe Grand)](https://www.youtube.com/watch?v=o5IySpAkThg).
Although technically still deterministic, the entropy introduced by drawing, due to timing noise and user movement, is nearly impossible to replicate, making it highly unpredictable.

This is a proof of concept and mainly for fun. It should demonstrate how human-generated entropy can reinforce password randomness. 

## ⚙️How It Works
**User Input:** As you draw on the canvas with your mouse, the tool records timestamped (x, y) coordinate data.

**Hashing:** This data is serialized and hashed using SHA-256 to compress it into a fixed-length output.

**Character Mapping:** The resulting hash is mapped onto a customizable character set to produce a secure password.

## ⚠️Limitations
**Not Completely Non-Deterministic:** While user input adds a layer of unpredictability, the process is still deterministic.

**Entropy Depends on User:** If your drawing is simple, short, or repeated, the resulting entropy will be lower.

**Limited to 32 characters:** Using SHA-256 for compression with 8 bits per character, we can generate passwords with a maximum length of 32 characters (this could be adjusted by using different hashing algorithms).
