package ms777.explorercanvas.test;

import java.awt.BorderLayout;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.event.ActionEvent;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.io.File;
import java.util.Vector;

import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.ButtonGroup;
import javax.swing.JButton;
import javax.swing.AbstractAction;
import javax.swing.JCheckBox;
import javax.swing.JComponent;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JRadioButton;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.SwingUtilities;
import javax.swing.UIManager;
import javax.swing.border.TitledBorder;

import ms777.explorercanvas.ExplorerCanvas;



@SuppressWarnings("serial")
public class ExplorerCanvasFrame extends JFrame {
	
	private final static int BOTTOM_PANEL_HEIGHT = 180;
	
	private final static String[] asExtension = new String[]{"txt", "pdf", "h", "cpp", "odt"};
	private final static String[] asDateModified = new String[]{"(none)", "LastWeek", "LastMonth", "LastYear"};

	public ExplorerCanvasFrame(String string) {
		super(string);
	}

    private JPanel panelLeft;
    private JPanel panelBottom;
    private ExplorerCanvas explorerCanvas;
    
    private Box panelSearchScope;
    
    private JTextField tfSearchText;
    private Box panelSearchExtension;
    private Box boxDateModified;
    private JTextArea tfAQSString;
    int iQueryCount = 1;
    
    private final static int MAX_SEARCHSCOPE_PATHS = 3;
    private Vector<String> vstrSearchScope = new Vector<String>();
    private JLabel[] labSearchScope = new JLabel[MAX_SEARCHSCOPE_PATHS];
    
     
    private void updateAQSString() {
    	String sExt = "";
    	for (Component c: panelSearchExtension.getComponents()) {
    		if (c instanceof JCheckBox) {
    			JCheckBox cb = (JCheckBox) c;
    			if (cb.isSelected()) {
    				if (sExt=="") {
    					sExt = "System.FileExtension:" + cb.getText();
    				} else {
    					sExt = sExt + " OR System.FileExtension:" + cb.getText();
    				}
    			}
    		}
    	}
    	if (sExt !="") sExt = "(" + sExt + ")";
    	
    	String sDateModified = "";
    	for (Component c: boxDateModified.getComponents()) {
    		if (c instanceof JRadioButton) {
    			JRadioButton cb = (JRadioButton) c;
    			if (cb.isSelected()) {
    				if (!cb.getText().equalsIgnoreCase(asDateModified[0])) {
        				sDateModified = "System.DateModified:System.StructuredQueryType.DateTime#" + cb.getText();
    				}
    				break;
    			}
    		}
    	}
    	
    	String sText = tfSearchText.getText().replace("\"", "\"\"");
    	if (!sText.equals("")) sText = "\"" + sText + "\"";
    	
    	String sSqs = sExt + (sExt.equals("")?"":" ") + sDateModified +  (sDateModified.equals("")?"":" ") + sText;
    	tfAQSString.setText(sSqs);
    }
    
    private void addToSearchScope(String newSearchScopePath) {
	    for (JLabel lab: labSearchScope) lab.setText("");
	    labSearchScope[0].setText("(search complete index)");

    	if (newSearchScopePath == null) {
    		vstrSearchScope.clear();
    	} else {
    		vstrSearchScope.add(newSearchScopePath);
    		if (vstrSearchScope.size() > MAX_SEARCHSCOPE_PATHS) vstrSearchScope.remove(0); 
    		for (int i=0; i < vstrSearchScope.size(); i++) labSearchScope[i].setText(vstrSearchScope.get(i));
    	}
    }
    
    
    private JComponent[] aButton = new JComponent[]{
    		new JButton(new AbstractAction("browse: C:\\") {
    			@Override
    			public void actionPerformed(ActionEvent e) {
    				explorerCanvas.browseTo("C:\\");
    			}
    	    }),
    		new JButton(new AbstractAction("browse: user.home") {
    			@Override
    			public void actionPerformed(ActionEvent e) {
    				explorerCanvas.browseTo(System.getProperty("user.home"));
    			}
    	    }),
    		new JButton(new AbstractAction("browse: null") {
    			@Override
    			public void actionPerformed(ActionEvent e) {
    				explorerCanvas.browseTo(null);
    			}
    	    }),
    		new JButton(new AbstractAction("browse parent") {
    			@Override
    			public void actionPerformed(ActionEvent e) {
    				explorerCanvas.browseRelative(ExplorerCanvas.SBSP_PARENT);
    			}
    	    }),
    		new JButton(new AbstractAction("browse back") {
    			@Override
    			public void actionPerformed(ActionEvent e) {
    				explorerCanvas.browseRelative(ExplorerCanvas.SBSP_NAVIGATEBACK);
    			}
    	    }),
    		new JButton(new AbstractAction("browse forward") {
    			@Override
    			public void actionPerformed(ActionEvent e) {
    				explorerCanvas.browseRelative(ExplorerCanvas.SBSP_NAVIGATEFORWARD);
    			}
    	    }),
    		new JButton(new AbstractAction("getSelection") {
    			@Override
    			public void actionPerformed(ActionEvent e) {
    				String[] arRet = explorerCanvas.getSelectedPaths();
    				if (arRet.length == 0) {
    					System.err.println("java: getSelectedPaths is null");
    				} else {
    					for (int i=0; i < arRet.length; i++) {
        					System.err.println("java: getSelectedPath["+i+"]: " + arRet[i]);
    					}
    				}
    			}
    	    }),
    		new JButton(new AbstractAction("getAllPaths") {
    			@Override
    			public void actionPerformed(ActionEvent e) {
    				String[] arRet = explorerCanvas.getAllPaths();
    				if (arRet == null) {
    					System.err.println("java: getAllPaths is null");
    				} else {
    					for (int i=0; i < arRet.length; i++) {
        					System.err.println("java: getAllPath["+i+"]: " + arRet[i]);
    					}
    				}
    			}
    	    }),
  	    
    	    new JButton(new AbstractAction("search .pdf .txt last month") {
    			@Override
    			public void actionPerformed(ActionEvent e) {
    				String[] asScope = {System.getProperty("user.home")}; 
    				String sSQL = "(System.FileExtension:txt OR System.FileExtension:pdf) AND System.DateModified:System.StructuredQueryType.DateTime#LastMonth";

    				explorerCanvas.doSearch(asScope, sSQL, "myQuery");
    			}
    	    }),
  	    
    	    new JButton(new AbstractAction("search pictures") {
    			@Override
    			public void actionPerformed(ActionEvent e) {
    				// http://msdn.microsoft.com/en-us/library/bb787521(v=vs.85).aspx
    				String[] asScope = {System.getProperty("user.home") + "\\pictures"};
    				explorerCanvas.doSearch(asScope, "System.Kind:System.Kind#picture", "myQuery");
    			}
    	    }),
  	    
    	    new JButton(new AbstractAction("search mails last month") {
    			@Override
    			public void actionPerformed(ActionEvent e) {
    				explorerCanvas.doSearch(null, "System.Kind:System.Kind#email AND System.Message.DateReceived:System.StructuredQueryType.DateTime#LastMonth", "myQuery");
    			}
    	    }),
  	    
    	    new JButton(new AbstractAction("command 1") {
    			@Override
    			public void actionPerformed(ActionEvent e) {
    				explorerCanvas.sendCommand(1);
    			}
    	    }),
  	    
    	    new JButton(new AbstractAction("command 2") {
    			@Override
    			public void actionPerformed(ActionEvent e) {
    				explorerCanvas.sendCommand(2);
    			}
    	    }),
  	    
    	    new JButton(new AbstractAction("command 3") {
    			@Override
    			public void actionPerformed(ActionEvent e) {
    				explorerCanvas.sendCommand(3);
    			}
    	    })
  	    
    };

   
    private void createGui() {
    	explorerCanvas = new ExplorerCanvas(ExplorerCanvas.MODE_BROWSER) {
    		@Override
    		public String[] getContextMenuCustomOptions(String sSelectedPath) {
    			File f = new File(sSelectedPath);
    			if (!f.exists()) return null;
				String sName = f.getAbsolutePath();
    			if (f.isDirectory()) {
        			return new String[]{"add " + sName + " to search scope", "do something with folder " + sName};
    			}
    			if (f.isFile()) {
        			return new String[]{"do something with file " + sName};
    			}
    			return null;
    		}
    		@Override
    		public void notifyContextMenuCustomOption(int iOption, final String sSelectedPath) {
    			System.err.println("java: notifyContextMenuCustomOption, sSelectedPath: " + sSelectedPath + ", iOption: " + iOption);
    			File f = new File(sSelectedPath);
 
    			if (!f.exists()) return;
    			if (f.isDirectory()) {
    				if (iOption==0) {
    					addToSearchScope(sSelectedPath);
    					return;
    				}
    				if (iOption==1) {
    					SwingUtilities.invokeLater(new Runnable() {
							@Override
							public void run() {
			       				JOptionPane.showMessageDialog(ExplorerCanvasFrame.this, "I did something with folder " + sSelectedPath);
 							}
    					});
    					return;
    				}
    			}
    			if (f.isFile()) {
    				if (iOption==0) {
    					SwingUtilities.invokeLater(new Runnable() {
							@Override
							public void run() {
			       				JOptionPane.showMessageDialog(ExplorerCanvasFrame.this, "I did something with file " + sSelectedPath);
 							}
    					});
    					return;
    				}
    			}
    			
    		}
    	};

       	panelLeft = new JPanel();
    	panelLeft.setLayout(new BoxLayout(panelLeft, BoxLayout.PAGE_AXIS));
    	panelLeft.setBorder(BorderFactory.createEmptyBorder(0, 10, 10, 10));

    	panelLeft.setPreferredSize(new Dimension(200, 100));
    	panelLeft.setMinimumSize(new Dimension(200, Integer.MAX_VALUE));
//    	panelLeft.setBackground(Color.YELLOW);

    	panelBottom = new JPanel(new GridBagLayout());
    	panelBottom.setMinimumSize(new Dimension(100, BOTTOM_PANEL_HEIGHT));
    	panelBottom.setMaximumSize(new Dimension(0, BOTTOM_PANEL_HEIGHT));
    	panelBottom.setPreferredSize(new Dimension(0, BOTTOM_PANEL_HEIGHT));
    	panelBottom.setBorder(new TitledBorder("text search panel"));
    	GridBagConstraints gbcPanelBottom = new GridBagConstraints();


    	
     	getContentPane().add(panelLeft, BorderLayout.LINE_START);
		getContentPane().add(explorerCanvas, BorderLayout.CENTER);
		getContentPane().add(panelBottom, BorderLayout.PAGE_END);
		
// the buttons		
		for (int i=0; i < aButton.length; i++) {
			panelLeft.add(Box.createRigidArea(new Dimension(0,5)));
			panelLeft.add(aButton[i]);
		}
// the bottom panel
		panelSearchExtension = new Box(BoxLayout.Y_AXIS);
		panelSearchExtension.setBorder(new TitledBorder("limit by extension"));
		panelSearchExtension.setAlignmentY(0.f);
		for (int i=0; i < asExtension.length; i++) {
			panelSearchExtension.add(new JCheckBox(new AbstractAction(asExtension[i]) {
				@Override
				public void actionPerformed(ActionEvent e) {
					updateAQSString();
				}
		    }));
		}
        panelSearchExtension.setMinimumSize(new Dimension(120, 0));
        panelSearchExtension.setPreferredSize(new Dimension(120, 0));
        gbcPanelBottom.gridx = 0;
        gbcPanelBottom.gridy = 0;
        gbcPanelBottom.gridwidth = 1;
        gbcPanelBottom.gridheight = GridBagConstraints.REMAINDER;
        gbcPanelBottom.weightx = 0.;
        gbcPanelBottom.fill = GridBagConstraints.VERTICAL;
        gbcPanelBottom.anchor = GridBagConstraints.FIRST_LINE_START;
		panelBottom.add(panelSearchExtension, gbcPanelBottom);
		
		boxDateModified = new Box(BoxLayout.Y_AXIS);
		boxDateModified.setBorder(new TitledBorder("modification date"));
		boxDateModified.setAlignmentY(0.f);
		ButtonGroup bgDateModified = new ButtonGroup();
		for (int i = 0; i < asDateModified.length; i++) {
			JRadioButton b = new JRadioButton(new AbstractAction(asDateModified[i]) {
				@Override
				public void actionPerformed(ActionEvent e) {
					updateAQSString();
				}
		    });
			if (i==0) b.setSelected(true);
			bgDateModified.add(b);
			boxDateModified.add(b);
		}
		boxDateModified.setMinimumSize(new Dimension(120, 0));
		boxDateModified.setPreferredSize(new Dimension(120, 0));
        gbcPanelBottom.gridx = 1;
        gbcPanelBottom.gridy = 0;
        gbcPanelBottom.gridwidth = 1;
        gbcPanelBottom.gridheight = GridBagConstraints.REMAINDER;
        gbcPanelBottom.weightx = 0.;
		panelBottom.add(boxDateModified, gbcPanelBottom);
		
		panelSearchScope = new Box(BoxLayout.Y_AXIS);
		panelSearchScope.setBorder(new TitledBorder("search scope"));
		panelSearchScope.setAlignmentY(0.f);

		panelSearchScope.add(new JButton(new AbstractAction("clear") {
			@Override
			public void actionPerformed(ActionEvent e) {
				addToSearchScope(null);
			}
	    	
	    }));
	    for (int i=0; i < labSearchScope.length; i++) {
	    	labSearchScope[i] = new JLabel();
	    	panelSearchScope.add(labSearchScope[i]);
	    }
		addToSearchScope(null);
		panelSearchScope.setMinimumSize(new Dimension(300, 0));
		panelSearchScope.setMaximumSize(new Dimension(300, 0));
		panelSearchScope.setPreferredSize(new Dimension(300, 0));
        gbcPanelBottom.gridx = 2;
        gbcPanelBottom.gridy = 0;
        gbcPanelBottom.gridwidth = 1;
        gbcPanelBottom.gridheight = GridBagConstraints.REMAINDER;
        gbcPanelBottom.weightx = 0.0;
		panelBottom.add(panelSearchScope, gbcPanelBottom);

		Box boxSearchText = new Box(BoxLayout.Y_AXIS);
		boxSearchText.setBorder(new TitledBorder("search text"));
		boxSearchText.setAlignmentY(0.f);
		boxSearchText.setAlignmentX(0.f);
        gbcPanelBottom.gridx = 3;
        gbcPanelBottom.gridy = 0;
        gbcPanelBottom.gridwidth = 1;
        gbcPanelBottom.gridheight = 1;
        gbcPanelBottom.weightx = 1.;
        gbcPanelBottom.fill = GridBagConstraints.HORIZONTAL;
		panelBottom.add(boxSearchText, gbcPanelBottom);

		tfSearchText = new JTextField();
		tfSearchText.addMouseListener(new MouseAdapter() {
			@Override
			public void mouseClicked(MouseEvent e) {
				explorerCanvas.giveupFocus();
			}
		});
		tfSearchText.addKeyListener(new KeyListener() {
			@Override
			public void keyTyped(KeyEvent e) {}
			@Override
			public void keyPressed(KeyEvent e) {}
			@Override
			public void keyReleased(KeyEvent e) {
				updateAQSString();
			}
		});
        boxSearchText.add(tfSearchText);
        
		Box boxAQSString = new Box(BoxLayout.Y_AXIS);
		boxAQSString.setBorder(new TitledBorder("AQS query:"));
        gbcPanelBottom.gridy = 1;
        gbcPanelBottom.fill = GridBagConstraints.HORIZONTAL;
        panelBottom.add(boxAQSString, gbcPanelBottom);

        
		tfAQSString = new JTextArea();
		tfAQSString.setRows(3);
		tfAQSString.setEditable(false);
		tfAQSString.setLineWrap(true);
		tfAQSString.setText("");
        boxAQSString.add(tfAQSString);
 
        gbcPanelBottom.gridy = 2;
        gbcPanelBottom.fill = GridBagConstraints.BOTH;
        panelBottom.add(new JButton(new AbstractAction("search") {
			@Override
			public void actionPerformed(ActionEvent e) {
				explorerCanvas.doSearch((String[]) vstrSearchScope.toArray(new String[]{}), tfAQSString.getText(), "myQuery" + iQueryCount++);
			}
	    	
	    }), gbcPanelBottom);
    }

	
	public static void main(String[] pArgs)  throws Exception {
		UIManager.setLookAndFeel("com.sun.java.swing.plaf.windows.WindowsLookAndFeel");
		ExplorerCanvasFrame frame = new ExplorerCanvasFrame("ExplorerCanvasFrame");
		frame.createGui();
		frame.setSize(800, 400);
		frame.pack();
		frame.setVisible(true);
		
		frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
	}
}
