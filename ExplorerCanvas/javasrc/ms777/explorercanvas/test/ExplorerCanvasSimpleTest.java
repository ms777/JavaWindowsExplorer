package ms777.explorercanvas.test;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import javax.swing.JFrame;

import ms777.explorercanvas.ExplorerCanvas;


@SuppressWarnings("serial")
public class ExplorerCanvasSimpleTest  {

    static class JExplorerCanvasFrame extends JFrame {
    	public final ExplorerCanvas canvas;
    	
		public JExplorerCanvasFrame(String string) {
			super(string);
			canvas = new ExplorerCanvas(ExplorerCanvas.MODE_BROWSER);
		    addWindowListener(new WindowAdapter() {
		    	@Override
		        public void windowClosing(WindowEvent e) {
		    		dispose();
//		            System.exit(-1);
		        }
		    });
		    getContentPane().add(canvas);
		    pack();
		    setVisible(true);
		    System.out.println("JExplorerCanvasFrame created, lHwnd: " + canvas.getHwnd());
		}
    	
    }
     
    
	public static void main(String[] args) throws Exception {
		final JExplorerCanvasFrame frame1 = new JExplorerCanvasFrame("ExplorerBrowser 1");

	    
	    final Thread thread = new Thread() {
	    	@Override
	    	public void run() {
	    		int i = 0;
	    		final JExplorerCanvasFrame frame2 = new JExplorerCanvasFrame("ExplorerBrowser 2");
	    	    try {Thread.sleep(2000);} catch (InterruptedException e) {e.printStackTrace();}
	    		frame2.canvas.browseTo("C:\\users");
	    	}
	    };
//	    Thread.sleep(1000);
		frame1.canvas.browseTo("C:\\users\\martin");
		System.out.println("browseto 1 finished");
//	    Thread.sleep(2000);
		final JExplorerCanvasFrame frame2 = new JExplorerCanvasFrame("ExplorerBrowser 2");
		frame2.canvas.browseTo("C:\\users");
		/*		
		System.out.println("browseto 2 finished");
		final JExplorerCanvasFrame frame3 = new JExplorerCanvasFrame("ExplorerBrowser 3");
		frame3.canvas.browseTo("C:\\users");
		System.out.println("browseto 3 finished");
	    Thread.sleep(2000);
*/	    
//		thread.start();
	}


}
