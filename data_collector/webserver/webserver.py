from flask import Flask, render_template, request, redirect, url_for, send_from_directory
import os

app = Flask(__name__)
UPLOAD_FOLDER = 'static/uploads'
app.config['UPLOAD_FOLDER'] = UPLOAD_FOLDER

def get_uploaded_files():
    return os.listdir(app.config['UPLOAD_FOLDER'])

@app.route('/')
def index():
    files = get_uploaded_files()
    # Sort file by name (descending)
    files.sort(reverse=True)
    return render_template('index.html', files=files)

@app.route('/upload', methods=['GET', 'POST'])
def upload_file():
    if request.method == 'POST':
        if 'file' not in request.files:
            return redirect(request.url)
        file = request.files['file']
        if file.filename == '':
            return redirect(request.url)
        if file:
            filename = file.filename
            file.save(os.path.join(app.config['UPLOAD_FOLDER'], filename))
    return redirect(url_for('index'))

@app.route('/uploads/<filename>')
def uploaded_file(filename):
    return render_template('uploaded.html', filename=filename)

@app.route('/download/<filename>', methods=['GET', 'POST'])
def download(filename):
    if request.method == 'POST':
        custom_name = request.form['custom_name']
        print(f'custom_name: {custom_name}')
        return send_from_directory(app.config['UPLOAD_FOLDER'], filename, as_attachment=True, download_name=custom_name)
    return redirect(url_for('uploaded_file', filename=filename))

# To delete a file
@app.route('/delete/<filename>', methods=['GET', 'POST'])
def delete(filename):
    if request.method == 'POST':
        os.remove(os.path.join(app.config['UPLOAD_FOLDER'], filename))
        return redirect(url_for('index'))

if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0', port=5000)

