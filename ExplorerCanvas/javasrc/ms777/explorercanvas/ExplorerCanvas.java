/*
 * Copyright 2014 Martin Schell

 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0

 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package ms777.explorercanvas;
import java.awt.Canvas;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.event.HierarchyEvent;
import java.awt.event.HierarchyListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.Date;
import java.util.Iterator;
import java.util.Map;
import java.util.jar.Attributes;
import java.util.jar.Manifest;
import java.util.prefs.Preferences;

import javax.swing.JFrame;
import javax.swing.JOptionPane;


@SuppressWarnings("serial")
public class ExplorerCanvas extends Canvas implements HierarchyListener {
	public final static int logERROR	= 0; 
	public final static int logWARNING	= 1; 
	public final static int logINFO		= 2; 
	public final static int logDEBUG	= 3; 
	public final static int logDEBUG1	= 4; 
	public final static int	logDEBUG2	= 5; 
	public final static int logDEBUG3	= 6; 
	public final static int logDEBUG4	= 7;
	
	public final static boolean B_GET_HWND_FROM_JAVA = true;
	
	static {
		if (!B_GET_HWND_FROM_JAVA) System.loadLibrary("jawt"); 
		
		String sJVMBitness = System.getProperty("os.arch");
		String sLibraryName = "libExplorerCanvas";
		if (sJVMBitness.equalsIgnoreCase("x86")) {
			sLibraryName = sLibraryName + "32";
		} else if (sJVMBitness.equalsIgnoreCase("amd64")) {
			sLibraryName = sLibraryName + "64";
		} else {
			throw new RuntimeException("system property os.arch(" + sJVMBitness + ") not recognized");
		}
		
		boolean isRunFromJar = ClassLoader.getSystemResource("") == null;
		
		try {
			if (isRunFromJar) {
				System.out.println("loading library " + sLibraryName + " from jar");
				loadLibFromJar("ExplorerCanvas", sLibraryName);
			} else {
				System.out.println("loading library " + sLibraryName + " through System.loadLibrary");
				System.loadLibrary(sLibraryName); 
			}
		} catch (UnsatisfiedLinkError e) {
			throw new RuntimeException("could not load library " + sLibraryName, e);
		}
		
//		setLoglevel(logDEBUG1);
		setLoglevel(logERROR);
	}


	//	EXPLORER_BROWSER_OPTIONS  
	public static final long EBO_NONE                = 0x00000000;
	public static final long EBO_NAVIGATEONCE        = 0x00000001;
	public static final long EBO_SHOWFRAMES          = 0x00000002;
	public static final long EBO_ALWAYSNAVIGATE      = 0x00000004;
	public static final long EBO_NOTRAVELLOG         = 0x00000008;
	public static final long EBO_NOWRAPPERWINDOW     = 0x00000010;
	public static final long EBO_HTMLSHAREPOINTVIEW  = 0x00000020;
	public static final long EBO_NOBORDER            = 0x00000040;
	public static final long EBO_NOPERSISTVIEWSTATE  = 0x00000080;

	//	FOLDERVIEWMODE
	public static final long FVM_AUTO        = -1;
	public static final long FVM_FIRST       = 1;
	public static final long FVM_ICON        = 1;
	public static final long FVM_SMALLICON   = 2;
	public static final long FVM_LIST        = 3;
	public static final long FVM_DETAILS     = 4;
	public static final long FVM_THUMBNAIL   = 5;
	public static final long FVM_TILE        = 6;
	public static final long FVM_THUMBSTRIP  = 7;
	public static final long FVM_CONTENT     = 8;

	// FOLDERFLAGS 
	public static final long FWF_NONE                 = 0x00000000;
	public static final long FWF_AUTOARRANGE          = 0x00000001;
	public static final long FWF_ABBREVIATEDNAMES     = 0x00000002;
	public static final long FWF_SNAPTOGRID           = 0x00000004;
	public static final long FWF_OWNERDATA            = 0x00000008;
	public static final long FWF_BESTFITWINDOW        = 0x00000010;
	public static final long FWF_DESKTOP              = 0x00000020;
	public static final long FWF_SINGLESEL            = 0x00000040;
	public static final long FWF_NOSUBFOLDERS         = 0x00000080;
	public static final long FWF_TRANSPARENT          = 0x00000100;
	public static final long FWF_NOCLIENTEDGE         = 0x00000200;
	public static final long FWF_NOSCROLL             = 0x00000400;
	public static final long FWF_ALIGNLEFT            = 0x00000800;
	public static final long FWF_NOICONS              = 0x00001000;
	public static final long FWF_SHOWSELALWAYS        = 0x00002000;
	public static final long FWF_NOVISIBLE            = 0x00004000;
	public static final long FWF_SINGLECLICKACTIVATE  = 0x00008000;
	public static final long FWF_NOWEBVIEW            = 0x00010000;
	public static final long FWF_HIDEFILENAMES        = 0x00020000;
	public static final long FWF_CHECKSELECT          = 0x00040000;
	public static final long FWF_NOENUMREFRESH        = 0x00080000;
	public static final long FWF_NOGROUPING           = 0x00100000;
	public static final long FWF_FULLROWSELECT        = 0x00200000;
	public static final long FWF_NOFILTERS            = 0x00400000;
	public static final long FWF_NOCOLUMNHEADER       = 0x00800000;
	public static final long FWF_NOHEADERINALLVIEWS   = 0x01000000;
	public static final long FWF_EXTENDEDTILES        = 0x02000000;
	public static final long FWF_TRICHECKSELECT       = 0x04000000;
	public static final long FWF_AUTOCHECKSELECT      = 0x08000000;
	public static final long FWF_NOBROWSERVIEWSTATE   = 0x10000000;
	public static final long FWF_SUBSETGROUPS         = 0x20000000;
	public static final long FWF_USESEARCHFOLDER      = 0x40000000;
	public static final long FWF_ALLOWRTLREADING      = 0x80000000;
	
	public static final long SVGIO_SELECTION          = 0x00000001;
	public static final long SVGIO_ALLVIEW            = 0x00000002;
	
	public static final int NAVIGATION_PENDING = 1;
	public static final int NAVIGATION_COMPLETE = 2;
	public static final int NAVIGATION_FAILED = 3;

	public final static int SBSP_PARENT = 0x00002000;
	public final static int SBSP_NAVIGATEBACK = 0x00004000;
	public final static int SBSP_NAVIGATEFORWARD = 0x00008000;
	
	public static class Mode {
		public final long lExplorerBrowserOptions;
		public final long lFolderViewMode;
		public final long lFolderFlags;
		public Mode(long lExplorerBrowserOptions, long lFolderViewMode, long lFolderFlags) {
			this.lExplorerBrowserOptions = lExplorerBrowserOptions;
			this.lFolderViewMode = lFolderViewMode;
			this.lFolderFlags = lFolderFlags;
		}
	}
	
	public final static Mode MODE_BROWSER = new Mode(EBO_NOWRAPPERWINDOW+EBO_NOBORDER, FVM_DETAILS, FWF_NONE);
	/* MODE_QUERY does not yet work. Always use MODE_BROWSER */ 
	public final static Mode MODE_QUERY = new Mode(EBO_NOBORDER, FVM_DETAILS, FWF_NOCLIENTEDGE);

	private native static synchronized void setLoglevel(int logLevel);
	private native static synchronized String getBuiltDate();
	
	private native static synchronized long notifyCreated(ExplorerCanvas explorerCanvas, long lExplorerBrowserOptions, long lFolderViewMode, long lFolderFlags);
	private native static synchronized void browseTo(ExplorerCanvas explorerCanvas, String sPath);
	private native static synchronized void sendCommand(ExplorerCanvas explorerCanvas, int iCommand);
	private native static synchronized void browseRelative(ExplorerCanvas explorerCanvas, int iSBSPconst);
	private native static synchronized String[] getSelectedPaths(ExplorerCanvas explorerCanvas, long svgio);
	private native static synchronized void setOptions(ExplorerCanvas explorerCanvas, long lOptions);
	private native static synchronized long doSearch(ExplorerCanvas explorerCanvas, String[] asScope, String sQuery, String sDisplayName);
	private native static synchronized void giveupFocus(ExplorerCanvas explorerCanvas);
	
	private long lhwnd = 0;

	private long lExplorerBrowserOptions;
	private long lFolderViewMode;
	private long lFolderFlags;
	
	public ExplorerCanvas(Mode mode) {
		this(mode.lExplorerBrowserOptions, mode.lFolderViewMode, mode.lFolderFlags);
	}
	public ExplorerCanvas(long lExplorerBrowserOptions, long lFolderViewMode, long lFolderFlags) {
		this.lExplorerBrowserOptions = lExplorerBrowserOptions;
		this.lFolderViewMode = lFolderViewMode;
		this.lFolderFlags = lFolderFlags;
		addHierarchyListener(this);
	    setPreferredSize(new Dimension(600, 600));
	    setMinimumSize(new Dimension(600, 600));
	}
	
	/**
	 * logMessage is called from the ExplorerBrowser. It can be overwritten
	 * @param s the message
	 */
	public void logMessage(String s) {
		System.err.println("java: logMessage on hwnd: " + lhwnd + ": " + s);
	}

	/**
	 * selectionChanged is called from the ExplorerBrowser. It can be overwritten.
	 * use getSelectedItems to know, what is selected
	 */
	public void selectionChanged() {
		System.err.println("java: selectionChange on object: " + lhwnd);
	}

	/**
	 * navigationOccurred is called from the ExplorerBrowser. It can be overwritten.
	 * @param iMode NAVIGATION_PENDING, NAVIGATION_COMPLETE, or NAVIGATION_FAILED
	 * @param sPath the target path
	 */
	public void navigationOccurred(int iMode, String sPath) {
		System.err.println("java: navigationOccurred on object: " + lhwnd + ", mode: " + iMode + ", path: " + sPath);
	}
	
	/**
	 * getContextMenuCustomOptions is called from the ExplorerBrowser. It can be overwritten.
	 * @param sSelectedPath the path, where the right click occured
	 * @return the array of custom options strings to be inserted at the bottom of the context menu
	 */
	public String[] getContextMenuCustomOptions(String sSelectedPath) {
		return new String[]{"custom option 0", "custom option 1", "custom option 2"};
	}
	
	/**
	 * notifyContextMenuCustomOption is called from the ExplorerBrowser. It can be overwritten.
	 * @param iOption the zero based index of the context menu custom option
	 * @param sSelectedPath the path, where the right click occured
	 */
	public void notifyContextMenuCustomOption(int iOption, String sSelectedPath) {
		System.err.println("java: notifyContextMenuCustomOption on object: " + lhwnd + ", sSelectedPath: " + sSelectedPath + ", iOption: " + iOption);
	}
	
	

	/**
	 * only for development
	 * @param iCommand
	 */
	public void sendCommand(int iCommand) {
		System.err.println("java: sendCommand on object: " + lhwnd + ": " + iCommand);
		sendCommand(this, iCommand);
	}

	/**
	 * browseTo 
	 * @param sPath the target path
	 */
	public void browseTo(String sPath) {
		browseTo(this, sPath);
	}
	
	/**
	 * browseRelative 
	 * @param iSBSPconst SBSP_PARENT, SBSP_NAVIGATEBACK, or SBSP_NAVIGATEFORWARD
	 */
	public void browseRelative(int iSBSPconst) {
		browseRelative(this, iSBSPconst);
	}
	
	public String[] getSelectedPaths() {
		return getSelectedPaths(this, SVGIO_SELECTION);
	}
	
	public String[] getAllPaths() {
		return getSelectedPaths(this, SVGIO_ALLVIEW);
	}
	
	/**
	 * Not tested. Better start a new ExplorerBrowser, when you want to modify options
	 * @param lOptions a combination of the EXPLORER_BROWSER_OPTIONS
	 */
	public void setOptions(long lOptions) {
		setOptions(this, lOptions);
	}
	
	/**
	 * Performs a Windows search
	 * @param asScope an array of the folders you want to search. Use null, if you want to search the complete index
	 * @param sQuery an AQS query syntax string. See the ExplorerBrowserFrame example for usage
	 * @param sDisplayName somehow needed by Windows. For subsequent searches use different display names, otherwise the previous search is only updated
	 * @return
	 */
	public long doSearch(String[] asScope, String sQuery, String sDisplayName) {
		return doSearch(this, asScope, sQuery, sDisplayName);
	}

	/**
	 * The native IExplorerBrowser somehow catches all keyboard input. Call giveupFocus, if you want to enter text in the rest of your application.
	 * It does not harm to call giveupFocus too often.
	 */
	public void giveupFocus() {
		giveupFocus(this);
	}


	/**
	 * Do not modify or override. hierarchyChanged is called by Swing. notifyCreated informs the dll about the window
	 * handle of the java parent window.
	 * B_GET_HWND_FROM_JAVA defaults to false. The if clause will most likely be removed
	 */
	@Override
	public void hierarchyChanged(HierarchyEvent e) {
		if ((e.getChangeFlags() & HierarchyEvent.DISPLAYABILITY_CHANGED) != 0) {
			if (isDisplayable()) {
				if (B_GET_HWND_FROM_JAVA) {
					Object wComponentPeer = invokeMethod(this, "getPeer");
					if (wComponentPeer==null) throw new RuntimeException("ExplorerCanvas:hierarchyChanged:could not get peer");
					lhwnd = (Long) invokeMethod(wComponentPeer, "getHWnd");
					notifyCreated(this, lExplorerBrowserOptions, lFolderViewMode, lFolderFlags);
				} else {
					lhwnd = 0;
					lhwnd = notifyCreated(this, lExplorerBrowserOptions, lFolderViewMode, lFolderFlags);
				}
			} else {
				lhwnd = 0;
			}
		}
	}

	public long getHwnd() {
		return lhwnd;
	}


    /**
     * helper for the case B_GET_HWND_FROM_JAVA = true. Will most likely be removed.
     * @param o
     * @param methodName
     * @return
     */
    private static Object invokeMethod(Object o, String methodName) {
        Class c = o.getClass();
        for (Method m : c.getMethods()) {
            if (m.getName().equals(methodName)) {
				try {
					return m.invoke(o);
				} catch (IllegalAccessException | IllegalArgumentException	| InvocationTargetException e) {
					e.printStackTrace();
			        throw new RuntimeException("ExplorerCanvas:invokeMethod: error on method named '"+methodName+"' on class " + c);
				}
            }
        }
        throw new RuntimeException("ExplorerCanvas:invokeMethod: Could not find method named '"+methodName+"' on class " + c);
    }
    
 
    /**
     * Extracts the dll from the jar and puts it to a temp dir (C:\Users\somebody\AppData\Local\Temp\(tmppath).
     * If a mismatch between the jar built date and the dll built date is detected,
     * the user is informed. She should confirm deleting the dll. Then the program writes info to the registry and exits.
     * On the next run, the dll is deleted and forced to be extracted from the jar.
     * If everything runs fine the dll is loaded.
     * @param tmppath the path in java.io.tempdir
     * @param name the library name without .dll, e.g. libExplorerCanvas64
     * @throws UnsatisfiedLinkError
     */
    private static void loadLibFromJar(String tmppath, String name) throws UnsatisfiedLinkError {
    	name = name + ".dll";
    	try {
    		InputStream in = ClassLoader.class.getResourceAsStream("/" + name );
    		File fileOut = new File(System.getProperty("java.io.tmpdir") + "/" + tmppath + "/");
    		fileOut.mkdirs();
    		fileOut = new File(System.getProperty("java.io.tmpdir") + "/" + tmppath + "/" + name);
    		if (fileOut.exists()) if (getDeleteDll()) {
    			System.out.println("loadLibFromJar: deleting dll " + fileOut.toString());
    			System.out.println("loadLibFromJar: deleting dll success: " + fileOut.delete());
    		}
    		if (!fileOut.exists()) {
    			OutputStream out;
    			out = new FileOutputStream(fileOut);
    			int n;
    			final byte buffer [] = new byte [1024*4096];
    			while(-1 != (n = in.read(buffer))) {
    				out.write(buffer, 0, n);
    			}
    			in.close();
    			out.flush();
    			out.close();
    		}
    		try {
    			System.load(fileOut.toString());
    		} catch (Exception e) {
    			fileOut.delete();
        		e.printStackTrace();
        		throw new UnsatisfiedLinkError("loadLibFromJar: System.load failed");
        	}
    		
			System.out.println("loadLibFromJar: System.load success");
    		InputStream inManifest = ClassLoader.class.getResourceAsStream("/META-INF/MANIFEST.MF");
			System.out.println("loadLibFromJar: inManifest: " + inManifest);

			Manifest manifest = new Manifest(inManifest);

			String sJarBuiltDate = manifest.getMainAttributes().getValue("Built-Date");
			System.out.println("loadLibFromJar: jar built date: " + sJarBuiltDate);
			String sDllBuiltDate = ExplorerCanvas.getBuiltDate();
			System.out.println("loadLibFromJar: dll built date: " + sDllBuiltDate);
			
			if (!sJarBuiltDate.equalsIgnoreCase(sDllBuiltDate)) {
				String s = "Built date of jar (" + sJarBuiltDate + ") does not match with\nbuilt date of dll (" + sDllBuiltDate
						+ ")\ndll path: " + fileOut.toString() 
						+ "\n\nPress OK to delete dll file, CANCEL to ignore\nOK is recommended";
				int iResponse = JOptionPane.showConfirmDialog(null, s, "ExplorerCanvas loading error", JOptionPane.OK_CANCEL_OPTION);
				if (iResponse==JOptionPane.OK_OPTION) {
					setDeleteDll();
					System.exit(1);
				}
			}

    	} catch (Exception e) {
    		e.printStackTrace();
    		throw new UnsatisfiedLinkError("loadLibFromJar: Failed to load required DLL");
    	}
    }
    
    private static final String PREFKEY = "deleteDll";
    		
    private static void setDeleteDll() {
    	Preferences.userRoot().node(ExplorerCanvas.class.getName()).putBoolean(PREFKEY, true);
    }

    private static boolean getDeleteDll() {
    	Preferences prefs = Preferences.userRoot().node(ExplorerCanvas.class.getName());
       	boolean b = prefs.getBoolean(PREFKEY, false);
       	prefs.remove(PREFKEY);
       	return b;
    }
}
