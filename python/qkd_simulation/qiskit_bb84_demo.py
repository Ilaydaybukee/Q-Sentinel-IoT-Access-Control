"""
Q-Sentinel Qiskit BB84 Demo
---------------------------
This script demonstrates a simplified BB84 quantum key distribution workflow
using Qiskit circuits and the Aer simulator.

Important:
- This is a simulation, not real quantum communication.
- It is designed to support the Q-Sentinel IoT project story:
  RFID + IR optical channel + quantum-inspired secure key demo.
"""

from __future__ import annotations

import random
from dataclasses import dataclass
from typing import List

from qiskit import QuantumCircuit, transpile
from qiskit_aer import AerSimulator


Basis = str  # "Z" or "X"


@dataclass
class BB84Result:
    sifted_key_alice: str
    sifted_key_bob: str
    sample_size: int
    sifted_size: int
    qber: float
    session_key_preview: str
    eavesdropper_enabled: bool


def measure_qubit(bit: int, alice_basis: Basis, bob_basis: Basis, backend: AerSimulator) -> int:
    """Prepare a single BB84 qubit and measure it with a selected basis."""
    qc = QuantumCircuit(1, 1)

    # Alice prepares |0> or |1> in the computational basis.
    if bit == 1:
        qc.x(0)

    # Alice switches to diagonal basis when basis is X.
    if alice_basis == "X":
        qc.h(0)

    # Bob measures in X basis by applying H before computational measurement.
    if bob_basis == "X":
        qc.h(0)

    qc.measure(0, 0)

    compiled = transpile(qc, backend)
    job = backend.run(compiled, shots=1, memory=True)
    result = job.result()
    measured = result.get_memory()[0]

    return int(measured)


def run_bb84(sample_size: int = 128, eavesdropper: bool = False, seed: int | None = 42) -> BB84Result:
    if seed is not None:
        random.seed(seed)

    backend = AerSimulator()

    alice_bits: List[int] = [random.randint(0, 1) for _ in range(sample_size)]
    alice_bases: List[Basis] = [random.choice(["Z", "X"]) for _ in range(sample_size)]
    bob_bases: List[Basis] = [random.choice(["Z", "X"]) for _ in range(sample_size)]

    bob_bits: List[int] = []

    for bit, alice_basis, bob_basis in zip(alice_bits, alice_bases, bob_bases):
        if eavesdropper:
            eve_basis: Basis = random.choice(["Z", "X"])
            eve_bit = measure_qubit(bit, alice_basis, eve_basis, backend)
            bob_bit = measure_qubit(eve_bit, eve_basis, bob_basis, backend)
        else:
            bob_bit = measure_qubit(bit, alice_basis, bob_basis, backend)

        bob_bits.append(bob_bit)

    sifted_alice: List[int] = []
    sifted_bob: List[int] = []

    for a_bit, b_bit, a_basis, b_basis in zip(alice_bits, bob_bits, alice_bases, bob_bases):
        if a_basis == b_basis:
            sifted_alice.append(a_bit)
            sifted_bob.append(b_bit)

    if not sifted_alice:
        qber = 0.0
    else:
        mismatches = sum(1 for a, b in zip(sifted_alice, sifted_bob) if a != b)
        qber = mismatches / len(sifted_alice)

    key_alice = "".join(str(x) for x in sifted_alice)
    key_bob = "".join(str(x) for x in sifted_bob)

    return BB84Result(
        sifted_key_alice=key_alice,
        sifted_key_bob=key_bob,
        sample_size=sample_size,
        sifted_size=len(sifted_alice),
        qber=qber,
        session_key_preview=key_alice[:32],
        eavesdropper_enabled=eavesdropper,
    )


def print_result(result: BB84Result) -> None:
    print("Q-Sentinel Qiskit BB84 Demo")
    print("---------------------------")
    print(f"Samples sent:      {result.sample_size}")
    print(f"Sifted key length: {result.sifted_size}")
    print(f"Eavesdropper:      {result.eavesdropper_enabled}")
    print(f"QBER:              {result.qber:.2%}")
    print(f"Session key demo:  {result.session_key_preview}")

    if result.qber > 0.11:
        print("Channel status:    UNSAFE / Possible interception")
    else:
        print("Channel status:    ACCEPTABLE for demo")


if __name__ == "__main__":
    clean_channel = run_bb84(sample_size=128, eavesdropper=False, seed=7)
    print_result(clean_channel)

    print("\n")

    attacked_channel = run_bb84(sample_size=128, eavesdropper=True, seed=7)
    print_result(attacked_channel)
