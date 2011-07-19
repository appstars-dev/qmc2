package sourceforge.org.qmc2.options.editor.ui;

import java.io.File;
import java.util.Iterator;
import java.util.Set;

import org.eclipse.core.commands.operations.DefaultOperationHistory;
import org.eclipse.core.commands.operations.IOperationHistory;
import org.eclipse.core.commands.operations.IUndoContext;
import org.eclipse.core.commands.operations.UndoContext;
import org.eclipse.jface.action.IMenuListener;
import org.eclipse.jface.action.IMenuManager;
import org.eclipse.jface.action.MenuManager;
import org.eclipse.jface.dialogs.ErrorDialog;
import org.eclipse.jface.viewers.ColumnViewerEditor;
import org.eclipse.jface.viewers.FocusCellOwnerDrawHighlighter;
import org.eclipse.jface.viewers.TreeViewer;
import org.eclipse.jface.viewers.TreeViewerColumn;
import org.eclipse.jface.viewers.TreeViewerEditor;
import org.eclipse.jface.viewers.TreeViewerFocusCellManager;
import org.eclipse.jface.viewers.Viewer;
import org.eclipse.jface.viewers.ViewerFilter;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.ModifyEvent;
import org.eclipse.swt.events.ModifyListener;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.FileDialog;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Menu;
import org.eclipse.swt.widgets.Text;
import org.eclipse.swt.widgets.TreeColumn;

import sourceforge.org.qmc2.options.editor.model.DescriptableItem;
import sourceforge.org.qmc2.options.editor.model.QMC2TemplateFile;
import sourceforge.org.qmc2.options.editor.ui.actions.AddLanguageAction;
import sourceforge.org.qmc2.options.editor.ui.actions.AddSectionAction;
import sourceforge.org.qmc2.options.editor.ui.actions.RedoAction;
import sourceforge.org.qmc2.options.editor.ui.actions.UndoAction;

public class QMC2Editor extends Composite {

	private final TreeViewer viewer;

	public static final String EDITOR_ID = "sourceforge.org.qmc2.options.editor";

	private String selectedFile = null;

	private QMC2TemplateFile templateFile = null;

	private String filter = null;

	private IOperationHistory operationHistory = new DefaultOperationHistory();

	private IUndoContext undoContext = new UndoContext();

	public QMC2Editor(Composite parent) {
		super(parent, SWT.NONE);
		setLayout(new GridLayout(3, false));

		createFileChooser();
		createFilterAndSearch();

		viewer = new TreeViewer(this, SWT.SINGLE | SWT.V_SCROLL | SWT.H_SCROLL
				| SWT.BORDER);
		viewer.setContentProvider(new QMC2ContentProvider());
		viewer.getTree().setHeaderVisible(true);
		viewer.getTree().setLinesVisible(true);
		viewer.getTree().setLayoutData(
				new GridData(SWT.FILL, SWT.FILL, true, true, 3, 1));

		TreeViewerEditor.create(viewer, new TreeViewerFocusCellManager(viewer,
				new FocusCellOwnerDrawHighlighter(viewer)),
				new QMC2EditorActivationStrategy(viewer),
				ColumnViewerEditor.TABBING_HORIZONTAL
						| ColumnViewerEditor.TABBING_MOVE_TO_ROW_NEIGHBOR
						| ColumnViewerEditor.TABBING_VERTICAL
						| ColumnViewerEditor.KEYBOARD_ACTIVATION);

		viewer.addFilter(new ViewerFilter() {

			@Override
			public boolean select(Viewer viewer, Object parentElement,
					Object element) {
				boolean select = false;
				if (filter != null) {
					if (element instanceof DescriptableItem) {
						DescriptableItem item = (DescriptableItem) element;
						Set<String> languages = item.getLanguages();
						Iterator<String> iterator = languages.iterator();
						while (!select && iterator.hasNext()) {
							String lang = iterator.next();
							String description = item.getDescription(lang)
									.toLowerCase();
							;
							if (description == null
									|| !description.contains(filter)) {
								select = select || false;
							} else {
								select = select || true;
							}
						}
						if (item.getName().contains(filter)) {
							select = select || true;
						} else {
							select = select || false;
						}
					}
				} else {
					select = true;
				}
				return select;
			}
		});

		MenuManager manager = new MenuManager();
		manager.setRemoveAllWhenShown(true);
		manager.addMenuListener(new IMenuListener() {
			@Override
			public void menuAboutToShow(IMenuManager manager) {
				populateContextMenu(manager);
			}
		});
		Menu m = manager.createContextMenu(viewer.getControl());
		viewer.getTree().setMenu(m);

	}

	private void populateContextMenu(IMenuManager manager) {
		manager.add(new UndoAction(QMC2Editor.this));
		manager.add(new RedoAction(QMC2Editor.this));
		manager.add(new AddLanguageAction(QMC2Editor.this));
		manager.add(new AddSectionAction(QMC2Editor.this));
	}

	private void createFilterAndSearch() {
		GridData layoutData = new GridData(SWT.LEAD, SWT.TOP, false, false, 1,
				1);
		Label filterLabel = new Label(this, SWT.NONE);
		filterLabel.setText("Filter: ");
		filterLabel.setLayoutData(layoutData);

		layoutData = new GridData(SWT.FILL, SWT.TOP, true, false, 2, 1);
		final Text filterText = new Text(this, SWT.SEARCH);
		filterText.setMessage("Filter...");
		filterText.setLayoutData(layoutData);
		filterText.addModifyListener(new ModifyListener() {

			@Override
			public void modifyText(ModifyEvent arg0) {
				filter = filterText.getText() == null ? null : filterText
						.getText().trim().length() == 0 ? null : filterText
						.getText().toLowerCase();

				viewer.refresh();

			}
		});

	}

	private void createFileChooser() {

		GridData layoutData = new GridData(SWT.LEAD, SWT.TOP, false, false, 1,
				1);
		Label fileSelectionLabel = new Label(this, SWT.NONE);
		fileSelectionLabel.setText("Template File: ");
		fileSelectionLabel.setLayoutData(layoutData);

		final Text fileSelection = new Text(this, SWT.BORDER | SWT.SINGLE);
		layoutData = new GridData(SWT.FILL, SWT.TOP, true, false, 1, 1);
		fileSelection.setLayoutData(layoutData);
		fileSelection.addModifyListener(new ModifyListener() {

			@Override
			public void modifyText(ModifyEvent e) {
				selectedFile = fileSelection.getText();
				File f = new File(selectedFile);
				if (f.exists() && f.isFile() && f.canRead()) {
					try {
						templateFile = QMC2TemplateFile.parse(new File(
								selectedFile));
						createColumns(templateFile.getLanguages());
						operationHistory = new DefaultOperationHistory();
						viewer.setInput(templateFile);
					} catch (Exception e1) {
						ErrorDialog.openError(getShell(), "Error",
								"Unable to open template file", null);
					}
				}

			}
		});

		Button browse = new Button(this, SWT.PUSH);
		browse.setText("Browse...");
		layoutData = new GridData(SWT.TRAIL, SWT.TOP, false, false, 1, 1);
		browse.setLayoutData(layoutData);

		browse.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent e) {
				FileDialog dialog = new FileDialog(getShell(), SWT.OPEN);
				dialog.setFileName(selectedFile);
				String filename = dialog.open();
				if (filename != null) {
					selectedFile = filename;
					fileSelection.setText(filename);
				}
			}
		});

	}

	private void createColumns(Set<String> languages) {

		int KEYS_COLUMN_SIZE = 150;

		viewer.getTree().setRedraw(false);
		for (TreeColumn c : viewer.getTree().getColumns()) {
			c.dispose();
		}

		TreeViewerColumn c = new TreeViewerColumn(viewer, SWT.NONE);

		c.getColumn().setMoveable(false);
		c.getColumn().setText("Item");
		c.setLabelProvider(new QMC2LabelProvider(null));
		c.getColumn().setWidth(KEYS_COLUMN_SIZE);

		for (String lang : languages) {
			c = createColumn(viewer, lang, "us".equals(lang) ? 1 : -1);
		}

		viewer.getTree().setRedraw(true);
		int columnSize = (viewer.getTree().getSize().x - KEYS_COLUMN_SIZE)
				/ (viewer.getTree().getColumnCount() - 1);

		for (int i = 1; i < viewer.getTree().getColumnCount(); i++) {
			viewer.getTree().getColumn(i).setWidth(columnSize);
		}

	}

	public TreeViewerColumn createColumn(TreeViewer viewer, String lang,
			int columnIndex) {
		TreeViewerColumn c = new TreeViewerColumn(viewer, SWT.NONE, columnIndex);
		c.getColumn().setMoveable(false);
		c.getColumn().setText(lang);
		c.setLabelProvider(new QMC2LabelProvider(lang));
		c.setEditingSupport(new QMC2EditingSupport(this, lang));
		return c;
	}

	public TreeViewer getViewer() {
		return viewer;
	}

	public IOperationHistory getOperationHistory() {
		return operationHistory;
	}

	public String getCurrentFile() {
		return selectedFile;
	}

	public QMC2TemplateFile getTemplateFile() {
		return templateFile;
	}

	public IUndoContext getUndoContext() {
		return undoContext;
	}
}
