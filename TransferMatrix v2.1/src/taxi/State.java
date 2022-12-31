package taxi;

public class State
{
	public long steps;
	public byte length;
	public State horizontal;
	public State vertical;
	public int index;
	
	public State(long steps, byte length)
	{
		this.steps = steps;
		this.length = length;
	}
	
	@Override
	public String toString()
	{
	    if(length == 0) {
	        return "Origin";
	    }
	    String binary = "";
	    for(byte i = 0; i < length; i++)
	    {
	    	if(((steps >> i) & 1) == 1)
	    	{
	    		binary += "1";
	    	}
	    	else
	    	{
	    		binary += "0";
	    	}
	    }
//	    for (long i = (1L << (length - 1)); i > 0; i = i / 2) {
//	        binary += ((steps & i) == 1L) ? "1" : "0";
//	    }
	    return binary;
		
//		// Prints left to right, opposite of how it's stored.
//		String binaryString = "";
//		for(int i = 0; i < length; i++)
//		{
//			if(((steps >>> i) & 1) == 1)
//			{
//				binaryString += "V";
//			}
//			else
//			{
//				binaryString += "H";
//			}
//		}
//		return "Length: " + length + ", " + binaryString;
	}
}
