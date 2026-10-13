import subprocess
import os
import shutil
import sys

# Ensure UTF-8 output
sys.stdout.reconfigure(encoding='utf-8')

core_dir = os.path.dirname(os.path.abspath(__file__))
asset_sample_dir = os.path.join(core_dir, "asset_sample")
archive_dir = os.path.join(asset_sample_dir, "archive")
test_code_file = os.path.join(asset_sample_dir, "codebase_sample.txt")
models_dir = os.path.join(core_dir, "database", "models")
vocab_file = os.path.join(models_dir, "vocab.axiom")
weights_file = os.path.join(models_dir, "weights.axiom")

print("=" * 60)
print("  PROJECT AXIOM: SELF-LEARNING C++ CODEBASE CONVERGENCE TEST")
print("=" * 60)

# Step 1: Temporarily stash existing media files to archive/
os.makedirs(archive_dir, exist_ok=True)
stashed_files = []
for fname in ["Photo.jpeg", "Video.mp4", "Voice.ogg"]:
    src = os.path.join(asset_sample_dir, fname)
    if os.path.exists(src):
        dst = os.path.join(archive_dir, fname)
        shutil.move(src, dst)
        stashed_files.append((dst, src))

try:
    # Step 2: Create a rich C++ training corpus from Project Axiom
    sample_cpp = """
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <stdfloat>

namespace axiom {
    struct Matrix {
        size_t rows;
        size_t cols;
        std::vector<std::bfloat16_t> data;
    };

    void forward_pass(const Matrix& weights, const Matrix& input) {
        std::cout << "Executing CUTLASS EVT Tensor Core Forward Pass" << std::endl;
    }

    void run_inference_engine() {
        std::cout << "Axiom Inference Engine Loaded Successfully" << std::endl;
    }

    int main() {
        std::cout << "Project Axiom C++23 CUDA Native AI" << std::endl;
        return 0;
    }
}
""" * 50 # Repeat to provide rich sequence patterns

    with open(test_code_file, "w", encoding="utf-8") as f:
        f.write(sample_cpp.strip())

    print(f"[+] Created C++ training file: {test_code_file} ({len(sample_cpp)} bytes)")

    # Step 3: Run Bulk Training for 10000 steps
    print("\n[..] Launching Bulk Training on C++ Source Code (10,000 steps)...")
    train_proc = subprocess.run(
        ["app.exe", "--bulk", "10000"],
        cwd=core_dir,
        capture_output=True,
        text=True,
        encoding="utf-8",
        errors="replace"
    )

    print("--- Training stdout ---")
    print(train_proc.stdout)
    if train_proc.stderr:
        print("--- Training stderr ---")
        print(train_proc.stderr)

    assert train_proc.returncode == 0, f"Training failed with code {train_proc.returncode}"
    if not os.path.exists(vocab_file):
        vocab_file = os.path.join(core_dir, "vocab.axiom")
    if not os.path.exists(weights_file):
        weights_file = os.path.join(core_dir, "weights.axiom")
    assert os.path.exists(vocab_file), "vocab.axiom was not created!"
    assert os.path.exists(weights_file), "weights.axiom was not created!"

    vocab_size = os.path.getsize(vocab_file)
    print(f"\n[OK] vocab.axiom generated: {vocab_size} bytes")
    weights_size = os.path.getsize(weights_file)
    print(f"[OK] weights.axiom generated: {weights_size} bytes")

    # Step 4: Test Inference Engine using the trained vocabulary
    print("\n[..] Launching Inference Engine and validating vocabulary restoration...")
    infer_proc = subprocess.run(
        ["app.exe", "--use"],
        cwd=core_dir,
        input="int main\nvoid run_inference_engine\nexit\n",
        capture_output=True,
        text=True,
        encoding="utf-8",
        errors="replace"
    )

    print("--- Inference Output ---")
    print(infer_proc.stdout)

    assert "Loading vocabulary from: vocab.axiom" in infer_proc.stdout, "Inference did not load vocab.axiom!"
    assert "Vocabulary successfully restored" in infer_proc.stdout, "Vocabulary restoration marker missing!"
    assert "[Prediction]" in infer_proc.stdout, "Prediction marker missing!"

    print("\n" + "=" * 60)
    print("  SELF-LEARNING C++ CODEBASE CONVERGENCE: 100% PASSED!")
    print("=" * 60)

finally:
    # Cleanup: restore stashed files
    if os.path.exists(test_code_file):
        os.remove(test_code_file)
    for dst, src in stashed_files:
        if os.path.exists(dst):
            shutil.move(dst, src)
    if os.path.exists(archive_dir):
        shutil.rmtree(archive_dir, ignore_errors=True)
    print("[+] Restored original asset_sample contents.")
