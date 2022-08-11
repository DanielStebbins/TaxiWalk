// Record: N=47 in 53.5 seconds. Used 5,500 MB of memory.

package taxi;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.util.LinkedList;
import java.util.TreeMap;

public class TaxiWalk
{
	// The length of walk to enumerate.
	public static final int N = 47;
	
	// These constants relate to the previously calculated steps to the origin file.
	public static final int MAX_N = 100;
	public static final int DIM = 2 * MAX_N + 1;
	public static final int LATTICE_SIZE = DIM * DIM;
	public static int[] stepsToOrigin = new int[LATTICE_SIZE];
	
	// Used for building the automaton.
	public static TreeMap<String, State> automaton = new TreeMap<String, State>();
	public static LinkedList<State> untreated = new LinkedList<State>();
	
	public static long findEndpoint = 0;
	public static long hasLoop = 0;
	public static long reduce = 0;
	public static long contains = 0;
	
	public static void main(String args[])
	{
		// Read the steps to the origin from every point, which have been previously calculated and stored.
		try
		{
			BufferedReader reader = new BufferedReader(new FileReader(new File("StepsToOrigin100.txt")));
			String[] split = reader.readLine().split(" ");
			for(int i = 0; i < LATTICE_SIZE; i++)
			{
				stepsToOrigin[i] = Integer.parseInt(split[i]);
			}
		}
		catch (IOException e)
		{
			e.printStackTrace();
		}
		
		long startTime = System.currentTimeMillis();
		
		// Set up the automaton stuff.
		State genesis = new State("");
		automaton.put("", genesis);
		untreated.add(genesis);
		
		
		// Main automaton-generating code.
		State movedState = new State("");
		while(!untreated.isEmpty())
		{
			// Possible change removal method.
			State start = untreated.pop();
			
			// Try to take a horizontal step.		
			if(start.pattern.length() < 2 || !start.pattern.substring(start.pattern.length() - 2).equals("HV"))
			{
				movedState.pattern = start.pattern + "H";
				
				long time = System.currentTimeMillis();
				// Find end point.
				int endX = 0;
				int endY = 0;
				for(int i = 0; i < movedState.pattern.length(); i++)
				{
					if(movedState.pattern.charAt(i) == 'H')
					{
						endX += 1 - 2 * Math.abs(endY % 2);
					}
					else
					{
						endY += 1 - 2 * Math.abs(endX % 2);
					}
				}
				findEndpoint += System.currentTimeMillis() - time;
				
				// Check for loop.		
				if(!hasLoop(movedState.pattern, endX, endY))
				{
					
					time = System.currentTimeMillis();
					// If cannot loop, chop off first step.
					while(stepsToOrigin[(endX + MAX_N) * DIM + endY + MAX_N] > N - movedState.pattern.length())
					{
						if(movedState.pattern.charAt(0) == 'H')
						{
							endX -= 1;
							endY = -endY;
						}
						else
						{
							endX = -endX;
							endY -= 1;
						}
						movedState.pattern = movedState.pattern.substring(1);
					}
					
					// If first step is now vertical, flip to horizontal. "ST" encoding faster?
					if(movedState.pattern.charAt(0) == 'V')
					{
						StringBuilder newPattern = new StringBuilder("");
						for(int i = 0; i < movedState.pattern.length(); i++)
						{
							if(movedState.pattern.charAt(i) == 'H')
							{
								newPattern.append('V');
							}
							else
							{
								newPattern.append('H');
							}
						}
						movedState.pattern = newPattern.toString();
					}
					reduce += System.currentTimeMillis() - time;
					
					
					time = System.currentTimeMillis();
					// Probably runs O(logn) search twice, bad but necessary if using TreeSet.
					State end = new State(movedState.pattern);
					State temp = automaton.putIfAbsent(movedState.pattern, end);
					if(temp != null)
					{
						start.horizontal = temp;
					}
					else
					{
						start.horizontal = end;
						untreated.add(end);
					}
					contains += System.currentTimeMillis() - time;
//					if(found)
//					{
//						// Present in tree.
//						start.horizontal = end;
//					}
//					else
//					{
//						// Absent from tree, add the temporary State.
//						time = System.currentTimeMillis();
//						end = new State(movedState.pattern);
//						automaton.add(end);
//						untreated.add(end);
//						start.horizontal = movedState;
//						add += System.currentTimeMillis() - time;
//					}
				}
			}
	
			
			// Try to take a vertical step.
			if(start.pattern.length() < 2 || !start.pattern.substring(start.pattern.length() - 2).equals("VH"))
			{
				movedState.pattern = start.pattern + "V";
				
				long time = System.currentTimeMillis();
				// Find end point.
				int endX = 0;
				int endY = 0;
				for(int i = 0; i < movedState.pattern.length(); i++)
				{
					if(movedState.pattern.charAt(i) == 'H')
					{
						endX += 1 - 2 * Math.abs(endY % 2);
					}
					else
					{
						endY += 1 - 2 * Math.abs(endX % 2);
					}
				}
				findEndpoint += System.currentTimeMillis() - time;
				
				// Check for loop.
				if(!hasLoop(movedState.pattern, endX, endY))
				{
					time = System.currentTimeMillis();
					// If cannot loop, chop off first step.
					while(stepsToOrigin[(endX + MAX_N) * DIM + endY + MAX_N] > N - movedState.pattern.length())
					{
						if(movedState.pattern.charAt(0) == 'H')
						{
							endX -= 1;
							endY = -endY;
						}
						else
						{
							endX = -endX;
							endY -= 1;
						}
						movedState.pattern = movedState.pattern.substring(1);
					}
					
					// If first step is now vertical, flip to horizontal. "ST" encoding faster?
					if(movedState.pattern.charAt(0) == 'V')
					{
						StringBuilder newPattern = new StringBuilder("");
						for(int i = 0; i < movedState.pattern.length(); i++)
						{
							if(movedState.pattern.charAt(i) == 'H')
							{
								newPattern.append('V');
							}
							else
							{
								newPattern.append('H');
							}
						}
						movedState.pattern = newPattern.toString();
					}
					reduce += System.currentTimeMillis() - time;
					
					time = System.currentTimeMillis();
					State end = new State(movedState.pattern);
					State temp = automaton.putIfAbsent(movedState.pattern, end);
					if(temp != null)
					{
						start.vertical = temp;
					}
					else
					{
						start.vertical = end;
						untreated.add(end);
					}
					contains += System.currentTimeMillis() - time;
				}
			}
		}
		long endTime = System.currentTimeMillis();
		System.out.println("N: " + N);
		System.out.println(automaton.size());
		System.out.println((endTime - startTime) / 1000.0 + "\n");
		
		System.out.println("Find Endpoint: " + findEndpoint / 1000.0 + "(" + Math.round((double) findEndpoint / (endTime - startTime) * 1000) / 10.0 + "%)");
		System.out.println("Has Loop: " + hasLoop / 1000.0 + "(" + Math.round((double) hasLoop / (endTime - startTime) * 1000) / 10.0 + "%)");
		System.out.println("Reduce Pattern: " + reduce / 1000.0 + "(" + Math.round((double) reduce / (endTime - startTime) * 1000) / 10.0 + "%)");
		System.out.println("Tree Contains: " + contains / 1000.0 + "(" + Math.round((double) contains / (endTime - startTime) * 1000) / 10.0 + "%)");
	}
	
	public static boolean hasLoop(String pattern, int endX, int endY)
	{
		long time = System.currentTimeMillis();
		int x = 0;
		int y = 0;
		boolean loop = x == endX && y == endY;
		int i = 0;
		while(!loop && i < pattern.length() - 12)
		{					
			if(pattern.charAt(i) == 'H')
			{
				x += 1 - 2 * Math.abs(y % 2);
			}
			else
			{
				y += 1 - 2 * Math.abs(x % 2);
			}
			loop = x == endX && y == endY;
			i++;
		}
		hasLoop += System.currentTimeMillis() - time;
		return loop;
	}
}
