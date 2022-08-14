// Record: N=47 in 30.0 seconds.

package taxi;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.util.LinkedList;
import java.util.TreeMap;

public class TaxiWalk
{
	// The length of walk to enumerate. MAX 64 CURRENTLY (long encoding).
	public static final int N = 47;
	
	// These constants relate to the previously calculated steps to the origin file.
	public static final int MAX_N = 100;
	public static final int DIM = 2 * MAX_N + 1;
	public static final int LATTICE_SIZE = DIM * DIM;
	public static int[] stepsToOrigin = new int[LATTICE_SIZE];
	
	// Used for building the automaton.
	public static TreeMap<Pattern, State> automaton = new TreeMap<Pattern, State>();
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
		State genesis = new State(new Pattern(0L, (byte) 0));
		automaton.put(genesis.pattern, genesis);
		untreated.addLast(genesis);
		
		// Main automaton-generating code.
		State movedState = new State(new Pattern(0L, (byte) 0));
		while(!untreated.isEmpty())
		{
			State start = untreated.removeFirst();
			
			// Try to take a horizontal step.
			if(start.pattern.length < 2 || start.pattern.steps >>> (start.pattern.length - 2) != 0b10)
			{
				movedState.pattern.steps = start.pattern.steps;
				movedState.pattern.length = (byte) (start.pattern.length + 1);
				
				
				// Find end point.
				long time = System.currentTimeMillis();
				int endX = 0;
				int endY = 0;
				for(int i = 0; i < movedState.pattern.length; i++)
				{
					// Try flip for speed-up.
//					if((movedState.pattern.steps & (1L << i)) == 0)
					if((movedState.pattern.steps >>> i & 1) == 0)
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
					// If cannot loop, chop off first step.
					time = System.currentTimeMillis();
					while(stepsToOrigin[(endX + MAX_N) * DIM + endY + MAX_N] > N - movedState.pattern.length)
					{
						if((movedState.pattern.steps & 1) == 0)
						{
							endX -= 1;
							endY = -endY;
						}
						else
						{
							endX = -endX;
							endY -= 1;
						}
						movedState.pattern.steps = movedState.pattern.steps >>> 1;
						movedState.pattern.length--;
					}
					
					// If first step is now vertical, flip to horizontal. "ST" encoding faster?
					if((movedState.pattern.steps & 1) == 1)
					{
						movedState.pattern.steps = movedState.pattern.steps ^ ((1L << movedState.pattern.length) - 1);
					}
					reduce += System.currentTimeMillis() - time;
					
					
					time = System.currentTimeMillis();
					State end = new State(new Pattern(movedState.pattern.steps, movedState.pattern.length));
					// Runs 2logn comparisons, could be improved.
					State temp = automaton.putIfAbsent(end.pattern, end);
					if(temp != null)
					{
						// Present in tree.
						start.horizontal = temp;
					}
					else
					{
						// Added to tree.
						start.horizontal = end;
						untreated.add(end);
					}
					contains += System.currentTimeMillis() - time;
				}
			}
	
			
			// Try to take a vertical step.
			if(start.pattern.length < 2 || start.pattern.steps >>> (start.pattern.length - 2) != 0b01)
			{
				movedState.pattern.steps = start.pattern.steps | (1L << start.pattern.length);
				movedState.pattern.length = (byte) (start.pattern.length + 1);
				
				// Find end point.
				long time = System.currentTimeMillis();
				int endX = 0;
				int endY = 0;
				for(int i = 0; i < movedState.pattern.length; i++)
				{
					// Try flip for speed-up.
//					if((movedState.pattern.steps & (1L << i)) == 0)
					if((movedState.pattern.steps >>> i & 1) == 0)
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
					// If cannot loop, chop off first step.
					time = System.currentTimeMillis();
					while(stepsToOrigin[(endX + MAX_N) * DIM + endY + MAX_N] > N - movedState.pattern.length)
					{
						if((movedState.pattern.steps & 1) == 0)
						{
							endX -= 1;
							endY = -endY;
						}
						else
						{
							endX = -endX;
							endY -= 1;
						}
						movedState.pattern.steps = movedState.pattern.steps >>> 1;
						movedState.pattern.length--;
					}
					
					// If first step is now vertical, flip to horizontal. "ST" encoding faster?
					if((movedState.pattern.steps & 1) == 1)
					{
						movedState.pattern.steps = movedState.pattern.steps ^ ((1L << movedState.pattern.length) - 1);
					}
					reduce += System.currentTimeMillis() - time;
					
					
					time = System.currentTimeMillis();
					State end = new State(new Pattern(movedState.pattern.steps, movedState.pattern.length));
					// Runs 2logn comparisons, could be improved.
					State temp = automaton.putIfAbsent(end.pattern, end);
					if(temp != null)
					{
						// Present in tree.		
						start.vertical = temp;
					}
					else
					{
						// Added to tree.
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
		
		// Running the automaton.
//		TreeMap<State, Integer> current = new TreeMap<State, Integer>();
//		TreeMap<State, Integer> next = new TreeMap<State, Integer>();
//		current.put(genesis, 1);
//		for(int i = 1; i <= N; i++)
//		{
//			for(State start : current.keySet())
//			{
//				if(start.horizontal != null)
//				{
//					next.put(start.horizontal, next.getOrDefault(start.horizontal, 0) + current.get(start));
//				}
//				
//				if(start.vertical != null)
//				{
//					next.put(start.vertical, next.getOrDefault(start.vertical, 0) + current.get(start));
//				}
//			}
//			current = next;
//			next = new TreeMap<State, Integer>();
//		}
//
//		long count = 0;
//		for(State s : current.keySet())
//		{
//			count += current.get(s);
//		}
//		System.out.println("\n" + count);
	}
	
	public static boolean hasLoop(Pattern pattern, int endX, int endY)
	{
		long time = System.currentTimeMillis();
		int x = 0;
		int y = 0;
		boolean loop = x == endX && y == endY;
		int i = 0;
		while(!loop && i < pattern.length - 12)
		{					
			if((pattern.steps & (1L << i)) == 0)
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