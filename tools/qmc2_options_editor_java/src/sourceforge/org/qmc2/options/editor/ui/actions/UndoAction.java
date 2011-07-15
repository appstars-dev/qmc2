package sourceforge.org.qmc2.options.editor.ui.actions;

import org.eclipse.jface.action.Action;
import org.eclipse.swt.SWT;

import sourceforge.org.qmc2.options.editor.ui.QMC2Editor;

public class UndoAction extends Action {

	private final QMC2Editor editor;

	public UndoAction(QMC2Editor editor) {
		this.editor = editor;
		setAccelerator(SWT.MOD1 + 'Z');
		setText("&Undo");
	}

	@Override
	public boolean isEnabled() {
		return editor.getOperationStack() != null
				&& editor.getOperationStack().hasUndoOperations();
	}

	@Override
	public void run() {
		editor.getOperationStack().undo();
	}

}
